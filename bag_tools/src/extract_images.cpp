/// Copyright (c) 2012,
/// Systems, Robotics and Vision Group
/// University of the Balearican Islands
/// All rights reserved.
/// 
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///     * Redistributions of source code must retain the above copyright
///       notice, this list of conditions and the following disclaimer.
///     * Redistributions in binary form must reproduce the above copyright
///       notice, this list of conditions and the following disclaimer in the
///       documentation and/or other materials provided with the distribution.
///     * Neither the name of Systems, Robotics and Vision Group, University of 
///       the Balearican Islands nor the names of its contributors may be used 
///       to endorse or promote products derived from this software without 
///       specific prior written permission.
/// 
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
/// ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
/// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
/// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
/// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
/// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <ros/ros.h>

#include <boost/format.hpp>

#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/image_encodings.h> 
#include <image_proc/processor.h>
#include <camera_calibration_parsers/parse.h>

#include <opencv2/highgui/highgui.hpp> // for cv::imwrite

#include "bag_tools/image_bag_processor.h"

using namespace sensor_msgs::image_encodings; 
/**
 * Saves color images from raw input
 */
class ImageSaver
{
public:
  ImageSaver(const std::string& save_dir, const std::string& filetype, 
      const std::string& prefix) :
    save_dir_(save_dir), filetype_(filetype), prefix_(prefix), num_saved_(0)
  {}

  void save(const sensor_msgs::ImageConstPtr &img)
  {
    image_proc::ImageSet output;
    image_geometry::PinholeCameraModel camera_model; // empty, we don't need it here
    if (img->encoding == MONO8      || 
        img->encoding == MONO16     || 
        img->encoding == TYPE_32FC1 || 
        img->encoding == TYPE_32SC1 || 
        img->encoding == TYPE_8UC1  || 
        img->encoding == TYPE_8SC1  ||
        img->encoding == TYPE_16UC1 ||
        img->encoding == TYPE_16SC1 || 
        img->encoding == TYPE_64FC1)
    {
      if (!processor_.process(img, camera_model, output, image_proc::Processor::MONO))
      {
        std::cerr << "ERROR Processing image" << std::endl;
        return;
      }
      std::string filename = 
        boost::str(boost::format("%s/%s%lu.%s") 
            % save_dir_ 
            % prefix_ 
            % img->header.stamp.toNSec() 
            % filetype_);
      if (!cv::imwrite(filename, output.mono))
        ROS_ERROR_STREAM("ERROR Saving " << filename);
      else
      {
        ROS_DEBUG_STREAM("Saved " << filename);
        num_saved_++;
      }
    }
    else
    {
      if (!processor_.process(img, camera_model, output, image_proc::Processor::COLOR))
      {
        std::cerr << "ERROR Processing image" << std::endl;
        return;
      }
      std::string filename = 
        boost::str(boost::format("%s/%s%lu.%s") 
            % save_dir_ 
            % prefix_ 
            % img->header.stamp.toNSec() 
            % filetype_);
      if (!cv::imwrite(filename, output.color))
        ROS_ERROR_STREAM("ERROR Saving " << filename);
      else
      {
        ROS_DEBUG_STREAM("Saved " << filename);
        num_saved_++;
      }
    }

  }

private:

  image_proc::Processor processor_;
  std::string save_dir_;
  std::string filetype_;
  std::string prefix_;
  int num_saved_;
};

int main(int argc, char** argv)
{
  if (argc < 4)
  {
    std::cout << "Usage: " << argv[0] << " OUT_DIR FILETYPE IMAGE_TOPIC BAGFILE [BAGFILE...]" << std::endl;
    std::cout << "  Example: " << argv[0] << " /tmp jpg /stereo_down/left/image_raw bag1.bag bag2.bag" << std::endl;
    return 0;
  }

  std::string out_dir(argv[1]);
  std::string filetype(argv[2]);
  std::string image_topic(argv[3]);
  
  ros::Time::init();

  std::string prefix("image");

  ImageSaver saver(out_dir, filetype, prefix);
  bag_tools::ImageBagProcessor processor(image_topic);
  processor.registerCallback(boost::bind(&ImageSaver::save, saver, _1));

  for (int i = 4; i < argc; ++i)
    processor.processBag(argv[i]);

  return 0;
}

