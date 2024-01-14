import tensorflow as tf
import numpy as np
import keras
import requests
from flask import Flask
from flask import render_template, send_from_directory
from flask import request
from flask_cors import CORS
from os import environ

CAMERA_URL = environ.get('CAMERA_URL')
IMG_WIDTH = 320
IMG_HEIGHT = 240

#model = keras.models.load_model("ModelCheckpoint.epoch27-loss0.03-acc0.98.hdf5")
app = Flask(__name__)
CORS(app)

@app.route("/")
def main_page():
    return render_template('page.html', camera_url=CAMERA_URL)

@app.post("/infer")
def infer():
    data = request.get_data(False)
    data = map(lambda x: x[1], filter(lambda x: x[0] % 4 != 3, enumerate(data))) # Remove alpha channel from the image
    img = np.fromiter(data, dtype=np.uint8).reshape((IMG_HEIGHT, IMG_WIDTH, 3))
    print(img)
    return {
        'msg': "Done"
    }

@app.route('/model.json')
def model():
    return send_from_directory('', 'model.json')

@app.route('/group1-shard1of1.bin')
def weights():
    return send_from_directory('', 'group1-shard1of1.bin')

if __name__ == '__main__':
    app.run()