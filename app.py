from flask import Flask
from flask import render_template
from flask import request
from flask_cors import CORS
from os import environ
from flask import Flask, render_template, send_from_directory


CAMERA_URL = environ.get('CAMERA_URL')

app = Flask(__name__)
CORS(app)

@app.route("/")
def main_page():
    return render_template('page.html', camera_url=CAMERA_URL)

@app.post("/infer")
def infer():
    data = request.get_data(False)
    print(data)
    return {
        'test': "Test"
    }

@app.route('/model.json')
def model():
    return send_from_directory('', 'model.json')

@app.route('/group1-shard1of1.bin')
def weights():
    return send_from_directory('', 'group1-shard1of1.bin')

if __name__ == '__main__':
    app.run()