# Street-Smarts

## Lighting the way for intelligent street lights

A brief overview of this project can be found [here](https://docs.google.com/document/d/1V0KVWVK2IO_dIYkn-9MKaT0tUAIUiJwxPMUMRKNkiK4/edit?usp=sharing)

### Group members:

- Kye Burchard
- Babu Wanyeki
- Linnea Rylander
- Viktor Urvantsev

# Documentation

The project ended up including 5 separate pieces of code, working together to create the interconnected system. There are two server-side files in Python and three pieces of ESP32 code in C++ - for the lamp, responder, and the map demo.

## `emergencyRequest.py`

This is the piece of code that ties everything together. It's configured to accept 7 different types of requests, either updating or returning data from a central database for the 31 lamps. The database has a row for each lamp, and looks something like this:

| ID | status | timestamp |
|----|--------|-----------|
| 0  | IDLE   | 18:56     |
| 4  | FIRE   | 21:34     |
|... | ...    | ...       |
| 30 | IDLE   | 18:56     |

Here's an outline of the types of requests it can receive, and what it returns/updates with them.

### POST from Postman with 'type' == 'CREATE'

Upon receiving this command, the database will be deleted if it exists, and recreated from scratch. This is useful so we don't have to try/except catch in every other function, depending on the database status.

### POST from `lamp.ino` with a float

If the status posted from a lamp is a float rather than a string, the code sends a request to the smart lamp API to set the brightness to the number posted.

### POST from `lamp.ino` with string

If the status is a string, the code simply updates the entry in the database for it to reflect the current status. Additionally, if the status posted was FIRE, IDLE, or ACCEPTED, it sends a request to the smart lamp API to update the color accordingly.

### POST from `responder.ino`

This code allows the responder ESP32 module to be able to accept requets for help, and updates entries in the database to reflect that.

### GET from `lamp.ino`

Here, the ID of the lamp that is requesting is sent, and the function simply returns the current status of the lamp ID that sent the request.

### GET from `responder.ino`

This is the trickiest bit of code. First, it queries the database to find out if there are currently any emergencies. If it doesn't find any, it simply returns that there are "No emergencies currently." If there is an emergency, it checks to see if the path has already been calculated to the emergency and saved into the `path` table in the database. If so, it simply returns that path. If not, it sends a request to `pathfinder.py`, asking for the shortest path between the fire station and the location of the fire. Finally, it stores the calculated path in the `path` table of the database so that future GET requests don't have to wait for the entire pathfinder code to run, and can instead immediately return the already-calculated path for next time.

### GET from `ID_to_arrayNEW.ino`

This is the code to control the laser-cut demonstration board with 31 LEDs, so it needs to return each ID that needs to be lit up. The easiest way to parse this in C++ seemed to be just separate IDs by spaces rather than return a string of a Python list. To do so, the function calls the function that would do a GET from `responder.ino` and processes the returned path into IDs with spaces between, then returns that to the ESP32. 

## `lamp.ino`

The gist of this code is just to control the lamp via inputs of motion detection, voice recognition, and buttons, and then display the relevant information on the screen.

### motion detection

stuff

### voice detection

stuff

### Dimming control

When motion isn't detected for 10 seconds, `lamp.ino` sends a request to the server code to set the lamp brightness to 0.1. If it then detects motion again, it sends a request to set the brightness to 0.5 (100% brightness was way too bright for something so close to our eyes ha)
