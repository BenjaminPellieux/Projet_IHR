#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/compressed_image.hpp>
#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/opencv.hpp>

class VideoPublisher : public rclcpp::Node {
public:
    VideoPublisher() : Node("video_pub_node") {
        // Publisher sur le topic "compressed_video_frames"
        publisher_ = this->create_publisher<sensor_msgs::msg::CompressedImage>("video_frames", 10);

        // Configuration de la fréquence de publication (30 Hz)
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(33), // ~30 FPS
            std::bind(&VideoPublisher::publishCompressedFrame, this)
        );

        // Initialiser la capture vidéo
        cap_.open(0); // Périphérique vidéo par défaut (ex: /dev/video0)
        if (!cap_.isOpened()) {
            cap_.open(1);
            if (!cap_.isOpened()) {
                RCLCPP_ERROR(this->get_logger(), "Erreur : Impossible d'ouvrir le flux vidéo !");
                rclcpp::shutdown();
            }
        }
    }

private:
    void publishCompressedFrame() {
        cv::Mat frame;
        cap_ >> frame; // Capture un frame
        if (frame.empty()) {
            RCLCPP_WARN(this->get_logger(), "Frame vidéo vide !");
            return;
        }

        // Convertir l'image en JPEG
	cv::cvtColor(frame, frame,CV_BGR2RGB);
        std::vector<uchar> compressedData;
        std::vector<int> compressionParams = {cv::IMWRITE_JPEG_QUALITY, 80}; // Qualité JPEG (80%)
        cv::imencode(".jpg", frame, compressedData, compressionParams);

        // Créer un message ROS2 CompressedImage
        auto msg = std::make_shared<sensor_msgs::msg::CompressedImage>();
        msg->header.stamp = this->now();
        msg->header.frame_id = "camera_frame";
        msg->format = "jpeg";
        msg->data = compressedData;

        // Publier le message
        publisher_->publish(*msg);
    }

    rclcpp::Publisher<sensor_msgs::msg::CompressedImage>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    cv::VideoCapture cap_;
};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VideoPublisher>());
    rclcpp::shutdown();
    return 0;
}

