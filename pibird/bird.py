
import time
import numpy as np
import picamera
import RPi.GPIO as GPIO
from PIL import Image
from tflite_runtime.interpreter import Interpreter

OUT_PIN=17 #out put pin for gpio speaker


GPIO.setmode(GPIO.BCM)     #set gpio mode to BCM
GPIO.setup(OUT_PIN,GPIO.OUT) #set the gpio pin as output

# init Raspberry Pi Camera
camera = picamera.PiCamera()
camera.resolution = (224, 224)  # ML model expects 224x224 image


# specify paths to local file assets
path_to_labels = "birds-label.txt"
path_to_model = "birds-model.tflite"
path_to_image = "images/bird.jpg"

# confidence threshold at which you want to be notified of a new bird
prob_threshold = 0.4


def main():
    check_for_bird()




def check_for_bird():

    labels = load_labels()
    interpreter = Interpreter(path_to_model)
    interpreter.allocate_tensors()
    _, height, width, _ = interpreter.get_input_details()[0]['shape']

    camera.start_preview()  #camera preview
    time.sleep(2)  # give the camera 2 seconds to adjust light balance
    camera.capture(path_to_image)
    image = Image.open(path_to_image)
    results = classify_image(interpreter, image)
    label_id, prob = results[0]
    print("bird: " + labels[label_id])
    print("prob: " + str(prob))
    camera.stop_preview()
    GPIO.output(OUT_PIN,GPIO.LOW);
    if prob > prob_threshold:
        bird = labels[label_id]
        bird = bird[bird.find(",") + 1:]
        prob_pct = str(round(prob * 100, 1)) + "%"
        print(bird);
        GPIO.output(OUT_PIN,GPIO.HIGH);
        time.sleep(3)  
        GPIO.output(OUT_PIN,GPIO.LOW);



def load_labels():
    """ load labels for the ML model from the file specified """
    with open(path_to_labels, 'r') as f:
        return {i: line.strip() for i, line in enumerate(f.readlines())}


def set_input_tensor(interpreter, image):
    tensor_index = interpreter.get_input_details()[0]['index']
    input_tensor = interpreter.tensor(tensor_index)()[0]
    input_tensor[:, :] = image


def classify_image(interpreter, image, top_k=1):
    """ return a sorted array of classification results """
    set_input_tensor(interpreter, image)
    interpreter.invoke()
    output_details = interpreter.get_output_details()[0]
    output = np.squeeze(interpreter.get_tensor(output_details['index']))

    # if model is quantized (uint8 data), then dequantize the results
    if output_details['dtype'] == np.uint8:
        scale, zero_point = output_details['quantization']
        output = scale * (output - zero_point)

    ordered = np.argpartition(-output, top_k)
    return [(i, output[i]) for i in ordered[:top_k]]





while True:
    main()
