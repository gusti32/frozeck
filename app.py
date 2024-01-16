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

app = Flask(__name__)
infer_model = keras.saving.load_model("ModelCheckpoint.epoch30-loss0.01-acc1.00.hdf5")
CORS(app)

print(type(infer_model))

@app.route("/")
def main_page():
    return render_template('page.html', camera_url=CAMERA_URL)

@app.post("/infer")
def infer():
    data = request.get_data(False)
    data = map(lambda x: x[1], filter(lambda x: x[0] % 4 != 3, enumerate(data))) # Remove alpha channel from the image
    img = np.fromiter(data, dtype=np.uint8).reshape((IMG_HEIGHT, IMG_WIDTH, 3)).astype(np.float32)
    img = tf.image.resize(img, size=(128, 128))
    img = np.expand_dims(img, axis=0)
    #print(type(infer_model))
    prediction = infer_model.predict(img)
    scores = tf.nn.softmax(prediction[0])
    predicted_class = np.argmax(scores)
    max_score = np.max(scores)
    class_name = ['Sehat', 'Tidak Sehat'][predicted_class]

    print("Prediction:", prediction)
    print("Softmax Scores:", scores.numpy())
    print("Predicted Class:", predicted_class)
    print("Max Score:", max_score)

    return {
        'result': class_name
    }

@app.route('/model.json')
def model():
    return send_from_directory('', 'model.json')

@app.route('/group1-shard1of1.bin')
def weights():
    return send_from_directory('', 'group1-shard1of1.bin')

if __name__ == '__main__':
    app.run()