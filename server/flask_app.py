import json
import os.path
import random
import string

from flask import Flask, render_template, request, session
from flask_bcrypt import Bcrypt
from flask_socketio import SocketIO

app = Flask(__name__)
bcrypt = Bcrypt(app)
app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app, async_mode='eventlet')

username = None
password = None
key = None

IR = {}
lights = {}

@app.route('/', methods = ["POST", "GET"])
def index():
  if request.method == "POST":
    if request.form["username"]==session["username"]\
     and request.form["password"]==session["password"]:
      key = random_string()
      return "key : {0}".format(key)
    else:
      return "Incorrect username or password. Press F5 and try again."
  else:
    return render_template('index.html')

@app.route("/register", methods = ["POST", "GET"])
def register():
  if (not os.path.isfile("file.dat")):
    if request.method == "POST":
      username = bcrypt.generate_password_hash(request.form["username"]).decode("utf-8") #remove '.decode("utf-8")' if you notice problems
      password = bcrypt.generate_password_hash(request.form["password"]).decode("utf-8")
      key = random_string()
      with open("file.dat", "w+") as f:
        f.truncate()
        json.dump([username,password,key], f)
      return "You registered successfully. Your key is {0}".format(key)
    else:
      return render_template('register.html')
  else:
    return "You already registered. Remove 'file.dat' and try again."

@app.route('/refresh', methods = ["POST", "GET"])
def refresh():
  if request.method == "POST":
    with open("file.dat", "r+") as f:
      x = json.load(f)
      username = x[0]
      password = x[1]
      if bcrypt.check_password_hash(username,request.form["username"])\
       and bcrypt.check_password_hash(password,request.form["password"]):
        key = random_string()
        y = [username,password,key]
        f.seek(0)
        f.truncate()
        json.dump(y,f)
        return "You refreshed successfully. Your new key is {0}".format(key)
      else:
        return "Incorrect username or password. Press F5 and try again." ##EDIT
  else:
    return render_template('refresh.html')

@app.route("/ifttt", methods = ["POST"])
def ifttt():
  user = request.get_json()["user"]
  iftttKey = request.get_json()["key"]
  deviceType = request.get_json()["type"]
  with open("file.dat", "r") as f:
    x = json.load(f)
    username = x[0]
    key = x[2]
    if bcrypt.check_password_hash(username,user) and\
     key == iftttKey:
      if deviceType == "IR":
        device = request.get_json()["device"]
        IR[device] = request.get_json()["action"]
        return "OK"
      elif deviceType == "light":
        device = request.get_json()["device"]
        IR[device] = request.get_json()["action"]
        return "OK"
    else:
      return "BAD"

@app.route("/device", methods = ["POST"])
def device():
  user = request.get_json()["user"]
  deviceKey = request.get_json()["key"]
  deviceType = request.get_json()["type"]
  with open("file.dat", "r") as f:
    x = json.load(f)
    username = x[0]
    key = x[2]
    if bcrypt.check_password_hash(username,user) and\
     key == deviceKey:
      if deviceType == "IR":
        device = request.get_json()["device"]
        pending = IR.pop(device,"None")
        return pending
      elif deviceType == "light":
        device = request.get_json()["device"]
        pending = IR.pop(device,"None")
        return pending
    else:
      return "BAD"
   
@socketio.on('connect', namespace='/test')
def test_connect():
  print('my response')
  return 'ok'

def random_string():
  secret_string = ''.join(random.SystemRandom().choice(string.ascii_uppercase\
   + string.digits) for _ in range(64))
  return secret_string


if __name__ == "__main__":
  socketio.run(app, host='0.0.0.0', debug = False, use_reloader=False)