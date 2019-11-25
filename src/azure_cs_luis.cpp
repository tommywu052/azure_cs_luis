//
// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.md file in the project root for full license information.
//
#define NOMINMAX
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 1
#define _CRT_SECURE_NO_WARNINGS

#include <ros/ros.h>
#include <windows.h>

#include <iostream>
#include <speechapi_cxx.h>
#include "std_msgs/String.h"

using namespace std;
using namespace Microsoft::CognitiveServices::Speech;

std::string g_luisKey;
std::string g_luisRegion;
ros::Publisher chatter_pub;

void recognizeSpeech()
{
    std::string trace;
    std_msgs::String msg;

    std::stringstream ss;
    
    // Creates an instance of a speech config with specified subscription key and service region.
    // Replace with your own subscription key and service region (e.g., "westus").
    auto config = SpeechConfig::FromSubscription("", "");

    // Creates a speech recognizer.
    auto recognizer = SpeechRecognizer::FromConfig(config);
    ROS_INFO("Waiting for utterance\n");

    // Performs recognition. RecognizeOnceAsync() returns when the first utterance has been recognized,
    // so it is suitable only for single shot recognition like command or query. For long-running
    // recognition, use StartContinuousRecognitionAsync() instead.
    auto result = recognizer->RecognizeOnceAsync().get();

    // Checks result.
    if (result->Reason == ResultReason::RecognizedSpeech)
    {
        ss << result->Text;;
        msg.data = ss.str();
        //trace = result->Text;
        cout << "We recognized: " << ss.str() << std::endl;
        //ROS_INFO(ss.str());
        chatter_pub.publish(msg);
        //ROS_INFO(trace.c_str());
    }
    else if (result->Reason == ResultReason::NoMatch)
    {
        cout << "NOMATCH: Speech could not be recognized." << std::endl;
        //ROS_INFO(trace.c_str());
    }
    else if (result->Reason == ResultReason::Canceled)
    {
        auto cancellation = CancellationDetails::FromResult(result);
        cout << "CANCELED: Reason=" << (int)cancellation->Reason << std::endl;

        if (cancellation->Reason == CancellationReason::Error) 
        {
            cout << "CANCELED: ErrorCode= " << (int)cancellation->ErrorCode << std::endl;
            cout << "CANCELED: ErrorDetails=" << cancellation->ErrorDetails << std::endl;
            cout << "CANCELED: Did you update the subscription info?" << std::endl;
        }
        //ROS_INFO(trace.c_str());
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "azure_cs_luis");
    ros::NodeHandle nh;

    g_luisKey = "";//std::getenv("azure_cs_luis_key");
    g_luisRegion = "";//std::getenv("azure_cs_luis_region");;
    chatter_pub = nh.advertise<std_msgs::String>("stt", 10);


    // Parameters.
    if (g_luisKey.empty() ||
        nh.getParam("azure_cs_luis_key", g_luisKey))
    {
        ROS_ERROR("luis key has not been set");
        nh.shutdown();
        return 0;
    }
    
    if (g_luisRegion.empty() ||
        nh.getParam("azure_cs_luis_region", g_luisRegion))
    {
        ROS_ERROR("luis region has not been set");
        nh.shutdown();
        return 0;
    }

    while (ros::ok())
    {
        recognizeSpeech();
        ros::spinOnce();
    }

    nh.shutdown();
    return 0;
}
