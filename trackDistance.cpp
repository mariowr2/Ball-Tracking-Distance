#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

/**

For using this program it is recommended to create a Resorces directory inside the project and store a "calibration image"
there. The calibration image should show the ball clearly. For the program to work, it must detect the ball in the provided 
"calibration image", see the calibration image for an example

For the program to work, it needs the following constants to be given values
int KNOWN_WIDTH   - the width of the ball we want to track
int KNOWN_DISTANCE - the known distance from the ball to the camera in the calibration image inside /Resources
string PATH - the path to the calibration image

int ballLowerOne/Two/Three  - this is the lower bound for detecting the color of the ball (hsv colorspace) in the image,
			      it is currently configured for detecting an orange ball in a bright setting
			      (roughly the color  a basketball)

int ballUpperOne/Two/Three - upper bound for detecting the color of the ball in hsv colorspace
			      

*/


//Constants
int KNOWN_WIDTH = 12;
int KNOWN_DISTANCE = 60;

string PATH="../Resources/image_name.jpg"; //PATH TO CALIBRATION IMAGE


double FOCAL_LENGTH = 0;  //starts at 0, modified later

int ballLowerOne = 0;
int ballLowerTwo = 159;
int ballLowerThree = 140;
                            // hsv color range for detecting the ball
int ballUpperOne = 56;
int ballUpperTwo = 255;
int ballUpperThree = 255;
//---------------------------------------------------------------------
//Prototypes
//---------------------------------------------------------------------
double findFocalLength(Mat);
vector<vector<Point>> manipulateAndFindContours(Mat);
double distanceToCamera(double);
vector<Point> findLargestContour(vector<vector<Point>>);
//---------------------------------------------------------------------

/* Note: the detecting window will only open up for the first time after the ball is in view, if it is not in view
 * when program starts, the window will not show up
 * the code in findFocalLength() and main() is almost the same, but because of all the checking in main
 * 
 * I did not manage to put that code into a single function conveniently.Exit video by pressing any key
 *
 */


//returns largest contour from a given list of contours
vector<Point> findLargestContour(vector<vector<Point>> contours){

    vector<Point> biggestContour;
    int largestArea = 0;
    int largestContourIndex = 0;

    for( int i = 0; i< contours.size(); i++ ) {

        double area=contourArea( contours[i],false); //loop through all contours and return the largest one after comparing them

        if(area>largestArea){
            largestArea=area;
            largestContourIndex=i;
        }
    }

    biggestContour = contours[largestContourIndex];
    return biggestContour;
}//find largest contour area

//finds focal length using a "calibration" image
double findFocalLength(Mat sampleImage){ //receives the calibration image and determines the focal length

    vector<vector<Point>> contours; //where vectors are stored
    vector<Point> biggestContour; //the biggest contour, the object we end up highlighting
    vector<Vec4i> hierarchy;
    double focalResult; //the value that will be returned
    cv::Point2f center;  //where the coordinates of the center of the ball will be stored
    float radius;       // the radius of the detected ball
    String windowName = "Calibration Image"; //Name of the window where calibration image will be shown

    contours = manipulateAndFindContours(sampleImage); //get list of contours
    biggestContour = findLargestContour(contours);    //get biggest contour from list
    minEnclosingCircle(biggestContour,center,radius); //get the min enclosing circle using our marker's contour
    focalResult = (2*radius*KNOWN_DISTANCE)/ KNOWN_WIDTH; //compute the focal length
    circle(sampleImage,center,radius,Scalar(0,255,255),2);  //draw a circle at location


    return focalResult;
}

//awkward name because most names that made sense already existed in the open cv library
//this method returns the contours found in an image using the global threshold variables
vector<vector<Point>> manipulateAndFindContours(Mat image){

    vector<vector<Point>> contours; //the list of contours that will be returned
    vector<Vec4i> hierarchy;
    Mat hsvImage;
    Mat maskImage;

    cvtColor(image,hsvImage,COLOR_BGR2HSV); //convert image to hsv
    inRange(hsvImage,Scalar(ballLowerOne,ballLowerTwo,ballLowerThree),Scalar(ballUpperOne,ballUpperTwo,ballUpperThree),maskImage); //crate the mask
    erode(maskImage,maskImage,NULL);
    dilate(maskImage,maskImage,NULL); //erode and dilate image to remove noise from it

    findContours(maskImage,contours,hierarchy,CV_RETR_TREE,CV_CHAIN_APPROX_SIMPLE); //find the contours

    return contours;
}

//computes distance to camera
double distanceToCamera(double perceivedWidth){
    return (KNOWN_WIDTH*FOCAL_LENGTH)/perceivedWidth;
}//distance to camera

int main() {

    Mat image = imread(PATH);
    FOCAL_LENGTH = findFocalLength(image);
    cout<< "The focal length is " <<FOCAL_LENGTH<<endl;
    namedWindow("Video"); //window where the frame is shown
    //after focal length is computed, the webcam's frames can be processed
    VideoCapture stream(0);

    if(!stream.isOpened()){
        cout<< "FAILED TO OPEN CAMERA" <<endl;
    }else{


     while(true){

         vector<vector<Point>> contours; //where vectors are stored
         vector<Point> biggestContour; //the biggest contour, the object we end up highlighting
         vector<Vec4i> hierarchy;
         double focalResult; //the value that will be returned
         cv::Point2f center;  //where the coordinates of the center of the ball will be stored
         float radius;       // the radius of the detected ball
         int distanceFromCamera;

         Mat webcamImage;
         stream.read(webcamImage); //grab the frame
         contours = manipulateAndFindContours(webcamImage);


         //only proceed if at least one contour is detected on the image
         if(contours.size() > 0 ){

             biggestContour = findLargestContour(contours);
             minEnclosingCircle(biggestContour,center,radius);

             if(radius > 10){

                 circle(webcamImage,center,radius,Scalar(0,255,255),2); //draw cirlce on image
                 distanceFromCamera = (int )distanceToCamera(2*radius); //get the distance from the camera
                 string distanceString = std::to_string(distanceFromCamera)+"CM"; //parse into string
                 putText(webcamImage,distanceString, Point2f(390,430), FONT_HERSHEY_SIMPLEX, 2,  Scalar(255,255,255,255)); //display text on image

                 //display image into window
                 imshow("Video", webcamImage); // show calibrated image.

                 if(waitKey(30) >= 0){
                     stream.release();
                     destroyAllWindows();
                     break;
                 }

             }//if radius is greater than 0


         }//if more than one contour detected



     }//infinite loop where video is captured


    }//else
    cout<<"END OF PROGRAM"<<endl;
    return 0;
}//main
