#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>

class ImageCompressor : public rclcpp::Node
{
public:
  ImageCompressor()
  : Node("image_compressor")
  {
    auto qos = rclcpp::QoS(10);
    qos.reliability(RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT);
    
    //publisher_ = this->create_publisher<sensor_msgs::msg::CompressedImage>("/sensing/camera/traffic_light/image_raw/compressed", qos);
    publisher_ = this->create_publisher<sensor_msgs::msg::CompressedImage>("/sensing/camera/traffic_light/image_raw/compressed", 10);
//    subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
//      "/sensing/camera/traffic_light/image_raw", 10, std::bind(&ImageCompressor::topic_callback, this, std::placeholders::_1));

    auto options = rclcpp::SubscriptionOptions();
    options.qos_overriding_options = rclcpp::QosOverridingOptions::with_default_policies();

    subscription_ = this->create_subscription<sensor_msgs::msg::Image>(
      "/sensing/camera/traffic_light/image_raw", qos, std::bind(&ImageCompressor::topic_callback, this, std::placeholders::_1), options);

  }


private:
  void topic_callback(const sensor_msgs::msg::Image::SharedPtr msg) const
  {
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
      return;
    }

    std::vector<int> params;
    params.push_back(cv::IMWRITE_JPEG_QUALITY);
    params.push_back(20);

    std::vector<uchar> buf;
    cv::imencode(".jpg", cv_ptr->image, buf, params);

    auto compressed_msg = sensor_msgs::msg::CompressedImage();
    compressed_msg.header = msg->header;
    compressed_msg.format = "jpeg";
    compressed_msg.data.assign(buf.begin(), buf.end());

    publisher_->publish(compressed_msg);
  }
  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscription_;
  rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr publisher_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<ImageCompressor>());
  rclcpp::shutdown();
  return 0;
}

