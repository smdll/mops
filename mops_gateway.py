import requests
from flask import Flask, request
from apscheduler import BackgroundScheduler
import pytz
import math
import time
from zeroconf import ServiceBrowser, Zeroconf

timezone = pytz.timezone("Asia/Shanghai")
app = Flask(__name__)
scheduler = BackgroundScheduler(timezone=timezone)
sunrise_time = {"hour": 7, "minute": 0}
sunset_time = {"hour": 18, "minute": 0}
zeroconf = Zeroconf()
listener = mDNSListener()
browser = ServiceBrowser(zeroconf, "_http._tcp.local.", listener)

class mDNSListener:
    def remove_service(self, zeroconf, type, name):
        print("Service %s removed" % (name,))

    def add_service(self, zeroconf, type, name):
        info = zeroconf.get_service_info(type, name)
        print("Service %s added, service info: %s" % (name, info))

#TODO: have a list of mops switches to iterate
def sunrise_task():
    requests.post(
        url="http://MopsSwitch.local/switch",
        data={"accesscode":"", "status":1}
    )

def sunset_task():
    requests.post(
        url="http://MopsSwitch.local/switch",
        data={"accesscode":"", "status":0}
    )

@scheduler.scheduled_job(trigger="cron", day_of_week=0, hour=4, minute=0)
def setSwitchTask():
    global sunrise_time, sunset_time

    # Get geolocation
    r = requests.get(
        url="https://api.map.baidu.com/location/ip",
        params={"ak": "HQi0eHpVOLlRuIFlsTZNGlYvqLO56un3", "coor":"bd09ll"}
    )
    if r is not None and r.status_code == 200:
        result = r.json()
        lon = float(result["content"]["point"]["x"])
        lat = float(result["content"]["point"]["y"])

        # Calculate sunrise and sunset time
        tz_offset = timezone._utcoffset.seconds/3600
        sunrise_time_float = 24/360*(180+tz_offset*15-lon-math.acos(-math.tan(
            -23.4*math.cos(2*math.pi*(time.localtime().tm_yday+9)/365)*math.pi/180
            )*math.tan(lat*math.pi/180))*180/math.pi)
        sunrise_time["hour"] = math.floor(sunrise_time_float)
        sunrise_time["minute"] = math.floor((sunrise_time_float-sunrise_time["hour"])*60)

        sunset_time_float = 24/360*(180+tz_offset*15-lon+math.acos(-math.tan(
            -23.4*math.cos(2*math.pi*(time.localtime().tm_yday+9)/365)*math.pi/180
            )*math.tan(lat*math.pi/180))*180/math.pi)
        sunset_time["hour"] = math.floor(sunset_time_float)
        sunset_time["minute"] = math.floor((sunset_time_float-sunset_time["hour"])*60)

    scheduler.reschedule_job(
        "sunrise_task", trigger="cron",
        hour=sunrise_time["hour"], minute=sunrise_time["minute"]
    )
    scheduler.reschedule_job(
        "sunset_task", trigger="cron",
        hour=sunset_time["hour"], minute=sunset_time["minute"]
    )

@app.route("/")
def root():
    return ""

if __name__ == "__main__":
    scheduler.add_job(
        sunrise_task, id="sunrise_task", trigger="cron",
        hour=sunrise_time["hour"], minute=sunrise_time["minute"]
    )
    scheduler.add_job(
        sunset_task, id="sunset_task", trigger="cron",
        hour=sunset_time["hour"], minute=sunset_time["minute"]
    )
    scheduler.start()
    setSwitchTask()
    app.run(host="127.0.0.1", port=8081, debug=False)