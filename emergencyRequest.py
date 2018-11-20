#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Sat Apr 21 18:34:31 2018

@author: linnea
@author2?: kyeb
"""

import json
import sqlite3
import datetime
import requests

token = "c5ce08a5a607e831ebe96f0b9337bd30c54ddf19e3286b97a267bdaa1bd1e86e"

headers = {
    "Authorization": "Bearer %s" % token,
}

payload = {
    "power": "on",
    "color": "white",
    "brightness": .5,
    "infared":0.0,
    "fast": "true",
    "duration":0.0,
}

db = "__HOME__/emergency.db"
# ID of the demo lamp
smolboiID = 4

def request_handler(request):
    if request['method'] == 'POST':
        data = json.loads(request['data'])
        if data['type'] == 'CREATE':
            return create_table()
        if data['type'] == 'lamp':
            if type(data['status']) == float:
                setLampBrightness(value=data['status'])
            else:
                return post_lamp(data)
        elif data['type'] == 'responder':
            return post_responder(data)
    if request['method'] == 'GET':
        values = request['values']
        if values['type'] == 'lamp':
            return get_lamp(request['values'])
        elif values['type'] == 'responder':
            return get_responder(request['values'])
        elif values['type'] == 'bigboi':
            return bigboi(values)


def bigboi(data):
    # basically what this should do is go over the database
    # and turn on all IDs in the box that are set to "ON"
    # in the database.
    x = get_responder(data)
    if x == "No emergencies currently":
        return ""
    outs = ""
    for digit in eval(x[x.index("\n") + 7:]):
        outs += str(digit) + " "
    return outs


def post_lamp(data):
    ID, status = data['ID'], data.get('status')
    conn = sqlite3.connect(db)
    c = conn.cursor()
    c.execute('''UPDATE lamps SET status=?, timing=? WHERE ID=?;''', (status,datetime.datetime.now(), ID))
    conn.commit()
    conn.close()
    if ID == smolboiID:
        if status == "FIRE":
            setLampColor('red')
        if status == "IDLE":
            setLampColor('white')
        if status == "ACCEPTED":
            setLampColor('blue')
    return "POSTed " + status + "!" + "\nLong press to cancel"


def post_responder(data):
    # insert some stuff here to figure out which lamps to update
    status, path = data['status'], data['path']
    path = eval(path)
    conn = sqlite3.connect(db)
    c = conn.cursor()
    ID = c.execute('''SELECT ID FROM lamps WHERE status = "FIRE"''').fetchall()
    ID = ID[0][0]
    c.execute('''UPDATE lamps SET status=?, timing=? WHERE ID=?;''', (status,datetime.datetime.now(), ID))
    for ID in path:
        c.execute('''UPDATE lamps SET status=?, timing=? WHERE ID=?;''', ("ON",datetime.datetime.now(), ID))
    conn.commit()
    conn.close()
    return "Request accepted"


def get_lamp(data):
    ID = data['ID']
    conn = sqlite3.connect(db)
    c = conn.cursor()
    things = c.execute('''SELECT status FROM lamps WHERE ID = ?;''', (ID,)).fetchall()
    output = things[0][0]
    conn.commit()
    conn.close()
    return output


def get_responder(data):
    outs = ""
    req_str = "http://iesc-s1.mit.edu/608dev/sandbox/linnear/pathfinder.py?start=13&end="
    conn = sqlite3.connect(db)
    c = conn.cursor()
    lamps = c.execute('''SELECT * FROM lamps;''').fetchall()
    path = c.execute('''SELECT * FROM path;''').fetchall()
    conn.commit()
    for lamp in lamps:
        if lamp[1] == "FIRE" or lamp[1] == "ACCEPTED":
            if not outs[-1:].isdigit():
                req_str += str(lamp[0])
            outs += "Fire at ID " + str(lamp[0]) + "\n"
            outs += "Path: "
    if outs == "":
        c.execute('''DROP TABLE path;''')
        c.execute('''CREATE TABLE path (path text);''')
        conn.commit()
        conn.close()
        return "No emergencies currently"
    elif len(path) > 0 and len(path[0][0]) > 0:
            path = path[0][0]
    else:
        r = requests.get(req_str)
        path = str(r.text)
        c.execute('''INSERT into path VALUES (?);''', (r.text,))
        conn.commit()
    conn.close()
    return outs + path


def create_table():
    conn = sqlite3.connect(db)
    c = conn.cursor()
    try:
        c.execute('''CREATE TABLE lamps (ID int, status text, timing timestamp);''')
        c.execute('''CREATE TABLE path (path text);''')
    except:
        c.execute('''DROP TABLE lamps;''')
        c.execute('''DROP TABLE path;''')
        c.execute('''CREATE TABLE path (path text);''')
        c.execute('''CREATE TABLE lamps (ID int, status text, timing timestamp);''')
    for ID in range(31):
        c.execute('''INSERT into lamps VALUES (?,?,?);''', (ID, "IDLE", datetime.datetime.now()))
    conn.commit()
    conn.close()
    return "table created with 31 rows"


def setLampColor(color="white"):
    # must be a generic color such as red, blue, pink, orange,etc..
    # default value is white
    payload["color"]=color
    requests.put('https://api.lifx.com/v1/lights/d073d5245413/state', data=payload, headers=headers)


def setLampBrightness(value=0.5):
    # pick a brightness value 0.0-1.0 must be a float.
    # default value is half brightness
    payload["brightness"]=value
    requests.put('https://api.lifx.com/v1/lights/d073d5245413/state', data=payload, headers=headers)


if __name__ == "__main__":
    pass
    # req_str = "http://iesc-s1.mit.edu/608dev/sandbox/linnear/pathfinder.py?start=13&end=4"
    # r = requests.get(req_str)
    # print(r.text)
