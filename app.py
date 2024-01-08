from flask import Flask
from flask import render_template
from os import environ

CAMERA_URL = environ.get('CAMERA_URL')

app = Flask(__name__)

@app.route("/")
def main_page():
    return render_template('page.html', camera_url=CAMERA_URL)

@app.route("/infer")
def infer():
    return 'Test'

if __name__ == '__main__':
    app.run()