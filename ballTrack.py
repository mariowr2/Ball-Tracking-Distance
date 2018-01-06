# import the necessary packages


import imutils
import cv2

# Focal Length: distance between the lens and the image sensor when the subject is in focus
# Triangle Similarity Formula.... F=(Pixel Width * Distance) / ObjectWidth

# Find focal length and then solve for distance in the while loop
# Distance formula: Distance = (Object Width * Focal Length) / Width in Pixels



#this program uses the imutils pyimagesearch , information about it can be found here: 
#https://www.pyimagesearch.com/2015/02/02/just-open-sourced-personal-imutils-package-series-opencv-convenience-functions/
#For using this program it is recommended to create a Resorces directory inside the project and store a "calibration image"
#there. The calibration image should show the ball clearly. For the program to work, it must detect the ball in the provided 
#"calibration image", see the calibration image for an example
#For the program to work, it needs the following constants to be given values
#int KNOWN_WIDTH   - the width of the ball we want to track
#int KNOWN_DISTANCE - the known distance from the ball to the camera in the calibration image inside /Resources
#string PATH - the path to the calibration image
#int ballLowerOne/Two/Three  - this is the lower bound for detecting the color of the ball (hsv colorspace) in the image,
#			      it is currently configured for detecting an orange ball in a bright setting
#			      (roughly the color  a basketball)
#int ballUpperOne/Two/Three - upper bound for detecting the color of the ball in hsv colorspace
			      




# Constants
KNOWN_WIDTH = 12
KNOWN_DISTANCE = 100
IMAGE_PATH = "../Resources/image_name.jpg"


FOCAL_LENGTH = 0  # modified later

ballLower = (0, 159, 140)   # bright setting
# ballLower = (0, 164, 91)  # dark setting
# ballLower = (0, 164, 91) #medium setting

ballUpper = (56, 255, 255)  # bright setting
# ballUpper = (192, 255, 255)    # upper medium setting
# ballUpper = (176, 255, 217)  # uppper dark setting


# ___________________________________________________________________________________


# find focal length
def find_focal():
    sampleImage = cv2.imread(IMAGE_PATH)
    frame = imutils.resize(sampleImage, width=600)  # resize
    contours = find_contours(frame)  # get list of contours in the image
    # ASSUME THAT CONTOUR WILL BE FOUND ON THE SAMPLE IMAGE
    c = max(contours, key=cv2.contourArea)  # get the largest contour
    ((x, y), radius) = cv2.minEnclosingCircle(c)  # find the centroid
    # now that I have the radius in pixels, we can calculate focal length

    print "focal length is "+str((2*radius*KNOWN_DISTANCE)/KNOWN_WIDTH)
    print "width in pixels is "+str(2*radius)

    cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)
    cv2.imshow("Calibration Image", frame)  # show the calibration image to the screen

    return (2*radius*KNOWN_DISTANCE)/KNOWN_WIDTH


# returns the distance to the camera
def distance_to_camera(focalLength, knownWidth, perWidth):
    return (knownWidth*focalLength)/perWidth


    # receives image input from camera, returns list of contour of the ball
def find_contours(originalFrame):
    hsv = cv2.cvtColor(originalFrame, cv2.COLOR_BGR2HSV)  # change image into hsv color space
    mask = cv2.inRange(hsv, ballLower, ballUpper)  # detect image and create a mask
    mask = cv2.erode(mask, None, iterations=2)    # process the image to remove noise from the image
    mask = cv2.dilate(mask, None, iterations=2)
    contours = cv2.findContours(mask.copy(), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)[-2]   #get list of contours
    return contours

# ______________________________________________________________________________________________________________________

FOCAL_LENGTH = find_focal() # find focal length


camera = cv2.VideoCapture(0)  # grab a reference to the camera
while True:

    (grabbed, frame) = camera.read()  # read a frame
    frame = imutils.resize(frame, width=600)  # resize
    contours = find_contours(frame)  # get list of contours in the image

    if len(contours) > 0:
        c = max(contours, key=cv2.contourArea)  # get the largest contour
        ((x, y), radius) = cv2.minEnclosingCircle(c)  # find the centroid
        if radius > 10:                                       # only proceed to draw circle if the radius meets minimum
            cv2.circle(frame, (int(x), int(y)), int(radius), (0, 255, 255), 2)

            distance = distance_to_camera(FOCAL_LENGTH, KNOWN_WIDTH, 2*radius)  # the distance is in inches

            cv2.putText(frame, str(int(distance))+" CM", (390, 430), cv2.FONT_HERSHEY_SIMPLEX, 2.0, (255, 255, 255), 3)


    cv2.imshow("Ball Detection", frame)   # show the frame to our screen
    key = cv2.waitKey(1) & 0xFF

    # quit program
    if key == ord("x"):
        break

# cleanup the camera and close any open windows
camera.release()
cv2.destroyAllWindows()

