#include "boost_emo_detector.h"
#include "matrix_io.h"
#include "gaborbank.h"
#include "facecrop.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <map>

using namespace std;
using namespace cv;
using namespace emotime;
  
const char * EmotionStrings[] = {
     "neutral",
     "anger",
     "contempt",
     "disgust",
     "fear",
     "happy",
     "sadness",
     "surprise"
  };

void help() {
	cout<<"Usage:"<<endl;
	cout<<"   boost_emo_detector_cli <image> <FaceDetecXML> <width> <height> <nwidths> <nlambdas> <nthetas> [<boostXML> ..] "<<endl;
	cout<<"Parameters:"<<endl;
	cout<<"   <image>       - The input image"<<endl;
	cout<<"   <faceDetectConf>   - OpenCV cascade classifier configuration file (Haar or LBP) for face detection"<<endl;
	cout<<"   <width>       - Width of the image, the input image will be scaled"<<endl;
	cout<<"   <height>      - Height of the image, the input image will be scaled"<<endl;
	cout<<"   <nwidths>     - "<<endl;
	cout<<"   <nlambdas>    - "<<endl;
	cout<<"   <nthetas>     - "<<endl;
	cout<<"   <boostXML>    - The trained boosted tree for detecting an expression "<<endl;
	cout<<"                   Name format: EMOTION_* where EMOTION is one of (neutral, contempt, disgust, fear, sadness, surprise)"<<endl;
	cout<<endl;
}
void banner() {
	cout<<"BoostEmoDetector Utility:"<<endl;
	cout<<"     Detect emotions using boosted trees"<<endl;
}

int main( int argc, const char *argv[] ) {
  if (argc < 2) {
		banner();
		help();
		cerr<<"ERR: missing parameters"<<endl;
		return -3;
	} 
	string infile = string(argv[1]);
	const char *config = argv[2];
  cv::Size size(0,0);
  int nwidths, nlambdas, nthetas;
  size.width = abs(atoi(argv[3]));
	size.height = abs(atoi(argv[4]));
  nwidths = abs(atoi(argv[5]));
  nlambdas= abs(atoi(argv[6]));
  nthetas = abs(atoi(argv[7]));
  vector<string> classifierPaths; 
  map<string, pair<emotime::Emotion,CvBoost> > classifiers; 
  
  if (argc > 9){
    // Read boost XML paths
    for (int i=8; i<argc;i++){
      classifierPaths.push_back(string(argv[i]));
    }  
  } else {
    cerr<<"ERR: you must specify some boosted trees"<<endl;
    return -2; 
  }

	try {

    // load classifiers and try to detect the emotion they have been trained to detect
    for (size_t i=0; i<classifierPaths.size();i++){
      string clpath= classifierPaths.at(i);
      size_t found = string::npos;
      CvBoost tree = CvBoost();
      tree.load(clpath.c_str());
      if ( (found=clpath.find(string(EmotionStrings[NEUTRAL])))==0){
        classifiers.insert( make_pair(string(EmotionStrings[NEUTRAL]), make_pair(NEUTRAL,tree)) );
      } else 
      if ( (found=clpath.find(string(EmotionStrings[ANGER])))==0){
        classifiers.insert( make_pair(string(EmotionStrings[ANGER]), make_pair(ANGER,tree)) );
      } else  
      if ( (found=clpath.find(string(EmotionStrings[CONTEMPT])))==0){
        classifiers.insert( make_pair(string(EmotionStrings[CONTEMPT]), make_pair(CONTEMPT,tree)) );
      } else  
      if ( (found=clpath.find(string(EmotionStrings[DISGUST])))==0){
        classifiers.insert( make_pair(string(EmotionStrings[DISGUST]), make_pair(DISGUST,tree)) );
      } else  
      if ( (found=clpath.find(string(EmotionStrings[FEAR])))==0){
        classifiers.insert( make_pair(string(EmotionStrings[FEAR]), make_pair(FEAR,tree)) );
      } else  
      if ( (found=clpath.find(string(EmotionStrings[HAPPY])))==0){
        classifiers.insert( make_pair(string(EmotionStrings[HAPPY]), make_pair(HAPPY,tree)) );
      } else  
      if ( (found=clpath.find(string(EmotionStrings[SADNESS])))==0){
        classifiers.insert( make_pair(string(EmotionStrings[SADNESS]), make_pair(SADNESS,tree)) );
      } else  
      if ( (found=clpath.find(string(EmotionStrings[SURPRISE])))==0){
        classifiers.insert( make_pair(string(EmotionStrings[SURPRISE]), make_pair(SURPRISE,tree)) );
      } 
    }

		Mat img = matrix_io_load(infile);
    Mat cropped;
		Mat scaled;
    Mat features;
    
    // Extract the face
    #ifdef DEBUG
    cerr<<"DEBUG: extracting face"<<endl;
    #endif
    
    FaceDetector facedetector=FaceDetector(config);
    bool hasFace=facecrop_cropFace( facedetector, img, cropped, true);
    if (!hasFace){
      cerr<<"ERR: cannot detect any face in image "<<infile<<endl;
      return -4;
    } 
		resize(cropped, scaled, size, 0, 0, CV_INTER_AREA);
    
    // Calculate feature vector
    #ifdef DEBUG
    cerr<<"DEBUG: creating feature vector"<<endl;
    #endif
    
    vector<struct GaborKern *> bank;
    gaborbank_getCustomGaborBank(bank, (double) nwidths, (double) nlambdas, (double) nthetas);
		features = gaborbank_filterImage(scaled, bank);
    features.reshape(1, 1);
    
    #ifdef DEBUG
    cerr<<"DEBUG: creating emo detector"<<endl;
    #endif

    EmoDetector<CvBoost> emodetector=boost_EmoDetector_create(classifiers);
    pair<Emotion,float> prediction= emodetector.predictMayorityOneVsAll(features);

    cout<<"Emotion predicted: "<<prediction.first<<" with score "<<prediction.second<<endl;

	} catch (int e) {
		cerr<<"ERR: Exception #"<<e<<endl;
		return -e;
	}
  return 0;
}