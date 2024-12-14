#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>

class VideoPublisher : public rclcpp::Node {
public:
    VideoPublisher() : Node("video_publisher") {
        // Publisher sur le topic "video_frames"
        publisher_ = this->create_publisher<sensor_msgs::msg::Image>("video_frames", 10);

        // Configuration de la fréquence de publication (30 Hz)
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(66), // ~30 FPS
            std::bind(&VideoPublisher::publishFrame, this)
        );

        // Initialiser la capture vidéo
        cap_.open(0); // Périphérique vidéo par défaut (ex: /dev/video0)
        if (!cap_.isOpened()) {
		cap_.open(1);
		if (! car_.isOpened()){
            		RCLCPP_ERROR(this->get_logger(), "Erreur : Impossible d'ouvrir le flux vidéo !");
            		rclcpp::shutdown();
		}
        }
    }

private:
    void publishFrame() {
        cv::Mat frame;
        cap_ >> frame; // Capture un frame
        if (frame.empty()) {
            RCLCPP_WARN(this->get_logger(), "Frame vidéo vide !");
            return;
        }

        // Convertir l'image OpenCV en message ROS2
        auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame).toImageMsg();
        publisher_->publish(*msg);
    }

    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    cv::VideoCapture cap_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VideoPublisher>());
    rclcpp::shutdown();
    return 0;
}

