// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"

// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>

#include <Eigen/Dense>
#include <Eigen/Core>
#include <math.h> 
// Timer
#include <chrono>
#include <float.h>
#include <iostream>  //declaring variables
#include <iomanip>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <unordered_map>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

// VertexBufferObject wrapper
vector<VertexBufferObject> brick_vertex_VBO_array;
vector<Eigen::MatrixXf> brick_Vertex_Array;
VertexBufferObject VBO;
VertexBufferObject paddle_vertex_VBO;
Eigen::MatrixXf paddle_vertex(4, 6);
VertexBufferObject ball_vertex_VBO;
Eigen::MatrixXf ready_vertex(4, 6);
VertexBufferObject ready_vertex_VBO;
Eigen::MatrixXf game_over_vertex(4, 6);
VertexBufferObject game_over_vertex_VBO;
Eigen::MatrixXf ball_vertex(4, 6);
VertexBufferObject background_vertex_VBO;
Eigen::MatrixXf background_vertex(4, 6);
Eigen::MatrixXf left_border_vertex(4, 6);
Eigen::MatrixXf right_border_vertex(4, 6);
Eigen::MatrixXf top_border_vertex(4, 6);
Eigen::Matrix4f identity(4,4);
Eigen::Matrix4f ball_translate(4,4);
Eigen::Matrix4f paddle_translate(4,4);
Eigen::Matrix4f ball_update(4, 4);
Eigen::Matrix2f rotate_matrix_neg_10(2, 2);
Eigen::Matrix2f rotate_matrix_pos_10(2, 2);
unordered_map<int, int> map;
vector<int> ball_grid;
vector<int> deleted_brick_index;
int move_multiplier = 0;
float brick_width = 44;
float brick_height = 20;
float paddle_height = 15;
float border_width = 18;
float brick_width_canonical;
float brick_height_canonical;
float paddle_height_canonical;
float paddle_width_canonical;
float side_border_width_canonical;
float top_border_width_canonical;
Eigen::Vector4f grid_topLeft;
Eigen::Vector4f grid_topLeft_canonical;
// Contains the vertex positions
Eigen::MatrixXf V(2,3);
Eigen:: Vector2f speed;
// Contains the view transformation
Eigen::Matrix4f view(4,4);
float speed_magnitude = 0.9;
bool game_start = false;
bool game_over = false;
bool win = false;
float degree;
float Sine;
float Cosine;

void collide(int index) {
    deleted_brick_index.push_back(index);
    if (speed[0] == 0) {
        speed[1] = -1 * speed[1];
    } else if (speed[1] == 0) {
        speed[0] = -1 * speed[0];
    } else if (speed[0] < 0 && speed[1] < 0) {
        if (brick_Vertex_Array.at(index).col(5)[1] - (ball_translate * ball_vertex.col(2))[1] > brick_Vertex_Array.at(index).col(5)[0] - (ball_translate * ball_vertex.col(2))[0]) {
            speed[0] = -1 * speed[0];
        } else {
            speed[1] = -1 * speed[1];
        }
    } else if (speed[0] < 0 && speed[1] > 0) {
        if ((ball_translate * ball_vertex.col(0))[1] - brick_Vertex_Array.at(index).col(1)[1] > brick_Vertex_Array.at(index).col(1)[0] - (ball_translate * ball_vertex.col(0))[0]) {
            speed[0] = -1 * speed[0];
        } else {
            speed[1] = -1 * speed[1];
        }
    } else if (speed[0] > 0 && speed[1] > 0) {
        if ((ball_translate * ball_vertex.col(5))[1] - brick_Vertex_Array.at(index).col(2)[1] > (ball_translate * ball_vertex.col(5))[0] - brick_Vertex_Array.at(index).col(2)[0]) {
            speed[0] = -1 * speed[0];
        } else {
            speed[1] = -1 * speed[1];
        }
    } else {
        if (brick_Vertex_Array.at(index).col(0)[1] - (ball_translate * ball_vertex.col(1))[1] > (ball_translate * ball_vertex.col(1))[0] - brick_Vertex_Array.at(index).col(0)[0]) {
            speed[0] = -1 * speed[0];
        } else {
            speed[1] = -1 * speed[1];
        }
    }

    for (int i = 0; i < deleted_brick_index.size(); i++) {
        cout << deleted_brick_index.at(i) << endl;
    }

}


void create_hashmap() {
    for (int i = 4; i < 12; i++) {
        for (int j = 1; j < 9; j++) {
            map[i * 10 + j] = (i - 4) * 8 + j - 1;
            cout << i * 10 + j << "--->" << map[i * 10 + j] << endl;
        }
    }
}

bool out_of_bound() {
    if ((ball_translate * ball_vertex.col(0))[1] < -1) {
        return true;
    } else {
        return false;
    }
}
void get_grid_number(float bottomLeft_x, float bottomLeft_y, float topRight_x, float topRight_y) {
    ball_grid.clear();

    int tl = int(-1 * (topRight_y - grid_topLeft_canonical[1]) / brick_height_canonical) * 10 + int((bottomLeft_x - grid_topLeft_canonical[0]) / brick_width_canonical);
    if (find(ball_grid.begin(), ball_grid.end(), tl) == ball_grid.end()) {
        ball_grid.push_back(tl);
    }
    int bl = int(-1 * (bottomLeft_y - grid_topLeft_canonical[1]) / brick_height_canonical) * 10 + int((bottomLeft_x - grid_topLeft_canonical[0]) / brick_width_canonical);
    if (find(ball_grid.begin(), ball_grid.end(), bl) == ball_grid.end()) {
        ball_grid.push_back(bl);
    }
    int tr = int(-1 * (topRight_y - grid_topLeft_canonical[1]) / brick_height_canonical) * 10 + int((topRight_x - grid_topLeft_canonical[0]) / brick_width_canonical);
    if (find(ball_grid.begin(), ball_grid.end(), tr) == ball_grid.end()) {
        ball_grid.push_back(tr);
    }
    int br = int(-1 * (bottomLeft_y - grid_topLeft_canonical[1]) / brick_height_canonical) * 10 + int((topRight_x - grid_topLeft_canonical[0]) / brick_width_canonical);
    if (find(ball_grid.begin(), ball_grid.end(), br) == ball_grid.end()) {
        ball_grid.push_back(br);
    }
    for (int i = 0; i < ball_grid.size(); i++) {
         cout << "grid number: " << ball_grid.at(i) << endl;
    }
    cout << "---------------------"<< endl;
}


void create_texture(GLenum buffer, unsigned int texture, const char* path){
    glGenTextures(1, &texture);
    glActiveTexture(buffer);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << path << std::endl;
    }else{
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void create_paddle(int width, int height) {
    Eigen::Vector4f topLeft(width / 2 - brick_width, paddle_height, 0, 1);
    Eigen::Vector4f topRight(width / 2 + brick_width, paddle_height, 0, 1);
    Eigen::Vector4f bottomLeft(width / 2 - brick_width, 0, 0, 1);
    Eigen::Vector4f bottomRight(width / 2 + brick_width, 0, 0, 1);
    Eigen::Vector4f canonical_topLeft((topLeft[0]/width)*2-1,(topLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_topRight((topRight[0]/width)*2-1,(topRight[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomLeft((bottomLeft[0]/width)*2-1,(bottomLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomRight((bottomRight[0]/width)*2-1,(bottomRight[1]/height)*2-1,0,1);
    /*Eigen::Vector4f world_topLeft = view.inverse()*canonical_topLeft;
    Eigen::Vector4f world_topRight = view.inverse()*canonical_topRight;
    Eigen::Vector4f world_bottomLeft = view.inverse()*canonical_bottomLeft;
    Eigen::Vector4f world_bottomRight = view.inverse()*canonical_bottomRight;*/

    paddle_vertex.col(0) = canonical_topLeft;
    paddle_vertex.col(1) = canonical_bottomRight;
    paddle_vertex.col(2) = canonical_bottomLeft;
    paddle_vertex.col(3) = canonical_topLeft;
    paddle_vertex.col(4) = canonical_bottomRight;
    paddle_vertex.col(5) = canonical_topRight;

    paddle_vertex_VBO.init();
    paddle_vertex_VBO.update(paddle_vertex);
    cout << paddle_vertex << endl;
}

void create_ready(int width, int height) {
    Eigen::Vector4f topLeft(width / 4, height * 4 / 5, 0, 1);
    Eigen::Vector4f topRight(width * 3 / 4, height * 4 / 5, 0, 1);
    Eigen::Vector4f bottomLeft(width / 4, height * 3 / 5, 0, 1);
    Eigen::Vector4f bottomRight(width * 3 / 4, height * 3 / 5, 0, 1);
    Eigen::Vector4f canonical_topLeft((topLeft[0]/width)*2-1,(topLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_topRight((topRight[0]/width)*2-1,(topRight[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomLeft((bottomLeft[0]/width)*2-1,(bottomLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomRight((bottomRight[0]/width)*2-1,(bottomRight[1]/height)*2-1,0,1);
    /*Eigen::Vector4f world_topLeft = view.inverse()*canonical_topLeft;
    Eigen::Vector4f world_topRight = view.inverse()*canonical_topRight;
    Eigen::Vector4f world_bottomLeft = view.inverse()*canonical_bottomLeft;
    Eigen::Vector4f world_bottomRight = view.inverse()*canonical_bottomRight;*/

    ready_vertex.col(0) = canonical_topLeft;
    ready_vertex.col(1) = canonical_bottomRight;
    ready_vertex.col(2) = canonical_bottomLeft;
    ready_vertex.col(3) = canonical_topLeft;
    ready_vertex.col(4) = canonical_bottomRight;
    ready_vertex.col(5) = canonical_topRight;

    ready_vertex_VBO.init();
    ready_vertex_VBO.update(ready_vertex);
    cout << paddle_vertex << endl;
}

void create_game_over(int width, int height) {
    Eigen::Vector4f topLeft(width / 4, height * 4 / 5, 0, 1);
    Eigen::Vector4f topRight(width * 3 / 4, height * 4 / 5, 0, 1);
    Eigen::Vector4f bottomLeft(width / 4, height * 3 / 5, 0, 1);
    Eigen::Vector4f bottomRight(width * 3 / 4, height * 3 / 5, 0, 1);
    Eigen::Vector4f canonical_topLeft((topLeft[0]/width)*2-1,(topLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_topRight((topRight[0]/width)*2-1,(topRight[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomLeft((bottomLeft[0]/width)*2-1,(bottomLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomRight((bottomRight[0]/width)*2-1,(bottomRight[1]/height)*2-1,0,1);
    /*Eigen::Vector4f world_topLeft = view.inverse()*canonical_topLeft;
    Eigen::Vector4f world_topRight = view.inverse()*canonical_topRight;
    Eigen::Vector4f world_bottomLeft = view.inverse()*canonical_bottomLeft;
    Eigen::Vector4f world_bottomRight = view.inverse()*canonical_bottomRight;*/

    game_over_vertex.col(0) = canonical_topLeft;
    game_over_vertex.col(1) = canonical_bottomRight;
    game_over_vertex.col(2) = canonical_bottomLeft;
    game_over_vertex.col(3) = canonical_topLeft;
    game_over_vertex.col(4) = canonical_bottomRight;
    game_over_vertex.col(5) = canonical_topRight;

    game_over_vertex_VBO.init();
    game_over_vertex_VBO.update(game_over_vertex);
    cout << paddle_vertex << endl;
}

void create_ball(int width, int height) {
    Eigen::Vector4f topLeft(width / 2 - brick_width / 6, paddle_height + brick_width / 3, 0, 1);
    Eigen::Vector4f topRight(width / 2 + brick_width / 6, paddle_height + brick_width / 3, 0, 1);
    Eigen::Vector4f bottomLeft(width / 2 - brick_width / 6, paddle_height, 0, 1);
    Eigen::Vector4f bottomRight(width / 2 + brick_width / 6, paddle_height, 0, 1);
    Eigen::Vector4f canonical_topLeft((topLeft[0]/width)*2-1,(topLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_topRight((topRight[0]/width)*2-1,(topRight[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomLeft((bottomLeft[0]/width)*2-1,(bottomLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomRight((bottomRight[0]/width)*2-1,(bottomRight[1]/height)*2-1,0,1);
    /*Eigen::Vector4f world_topLeft = view.inverse()*canonical_topLeft;
    Eigen::Vector4f world_topRight = view.inverse()*canonical_topRight;
    Eigen::Vector4f world_bottomLeft = view.inverse()*canonical_bottomLeft;
    Eigen::Vector4f world_bottomRight = view.inverse()*canonical_bottomRight;
*/
    ball_vertex.col(0) = canonical_topLeft;
    ball_vertex.col(1) = canonical_bottomRight;
    ball_vertex.col(2) = canonical_bottomLeft;
    ball_vertex.col(3) = canonical_topLeft;
    ball_vertex.col(4) = canonical_bottomRight;
    ball_vertex.col(5) = canonical_topRight;

    ball_vertex_VBO.init();
    ball_vertex_VBO.update(ball_vertex);
    cout << paddle_vertex << endl;
}

void create_background(int width, int height) {
    Eigen::Vector4f topLeft(0, height, 0, 1);
    Eigen::Vector4f topRight(width, height, 0, 1);
    Eigen::Vector4f bottomLeft(0, 0, 0, 1);
    Eigen::Vector4f bottomRight(width, 0, 0, 1);
    Eigen::Vector4f canonical_topLeft((topLeft[0]/width)*2-1,(topLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_topRight((topRight[0]/width)*2-1,(topRight[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomLeft((bottomLeft[0]/width)*2-1,(bottomLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomRight((bottomRight[0]/width)*2-1,(bottomRight[1]/height)*2-1,0,1);
    /*Eigen::Vector4f world_topLeft = view.inverse()*canonical_topLeft;
    Eigen::Vector4f world_topRight = view.inverse()*canonical_topRight;
    Eigen::Vector4f world_bottomLeft = view.inverse()*canonical_bottomLeft;
    Eigen::Vector4f world_bottomRight = view.inverse()*canonical_bottomRight;*/

    background_vertex.col(0) = canonical_topLeft;
    background_vertex.col(1) = canonical_bottomRight;
    background_vertex.col(2) = canonical_bottomLeft;
    background_vertex.col(3) = canonical_topLeft;
    background_vertex.col(4) = canonical_bottomRight;
    background_vertex.col(5) = canonical_topRight;

    background_vertex_VBO.init();
    background_vertex_VBO.update(background_vertex);
    cout << paddle_vertex << endl;
}

void create_left_border(int width, int height) {
    Eigen::Vector4f topLeft(0, height, 0, 1);
    Eigen::Vector4f topRight(border_width, height, 0, 1);
    Eigen::Vector4f bottomLeft(0, 0, 0, 1);
    Eigen::Vector4f bottomRight(border_width, 0, 0, 1);
    Eigen::Vector4f canonical_topLeft((topLeft[0]/width)*2-1,(topLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_topRight((topRight[0]/width)*2-1,(topRight[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomLeft((bottomLeft[0]/width)*2-1,(bottomLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomRight((bottomRight[0]/width)*2-1,(bottomRight[1]/height)*2-1,0,1);
    /*Eigen::Vector4f world_topLeft = view.inverse()*canonical_topLeft;
    Eigen::Vector4f world_topRight = view.inverse()*canonical_topRight;
    Eigen::Vector4f world_bottomLeft = view.inverse()*canonical_bottomLeft;
    Eigen::Vector4f world_bottomRight = view.inverse()*canonical_bottomRight;*/

    left_border_vertex.col(0) = canonical_topLeft;
    left_border_vertex.col(1) = canonical_bottomRight;
    left_border_vertex.col(2) = canonical_bottomLeft;
    left_border_vertex.col(3) = canonical_topLeft;
    left_border_vertex.col(4) = canonical_bottomRight;
    left_border_vertex.col(5) = canonical_topRight;
}

void create_right_border(int width, int height) {
    Eigen::Vector4f topLeft(width - border_width, height, 0, 1);
    Eigen::Vector4f topRight(width, height, 0, 1);
    Eigen::Vector4f bottomLeft(width - border_width, 0, 0, 1);
    Eigen::Vector4f bottomRight(width, 0, 0, 1);
    Eigen::Vector4f canonical_topLeft((topLeft[0]/width)*2-1,(topLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_topRight((topRight[0]/width)*2-1,(topRight[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomLeft((bottomLeft[0]/width)*2-1,(bottomLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomRight((bottomRight[0]/width)*2-1,(bottomRight[1]/height)*2-1,0,1);
    /*Eigen::Vector4f world_topLeft = view.inverse()*canonical_topLeft;
    Eigen::Vector4f world_topRight = view.inverse()*canonical_topRight;
    Eigen::Vector4f world_bottomLeft = view.inverse()*canonical_bottomLeft;
    Eigen::Vector4f world_bottomRight = view.inverse()*canonical_bottomRight;*/

    right_border_vertex.col(0) = canonical_topLeft;
    right_border_vertex.col(1) = canonical_bottomRight;
    right_border_vertex.col(2) = canonical_bottomLeft;
    right_border_vertex.col(3) = canonical_topLeft;
    right_border_vertex.col(4) = canonical_bottomRight;
    right_border_vertex.col(5) = canonical_topRight;
}

void create_top_border(int width, int height) {
    Eigen::Vector4f topLeft(border_width, height, 0, 1);
    Eigen::Vector4f topRight(width - border_width, height, 0, 1);
    Eigen::Vector4f bottomLeft(border_width, height - border_width, 0, 1);
    Eigen::Vector4f bottomRight(width - border_width, height - border_width, 0, 1);
    Eigen::Vector4f canonical_topLeft((topLeft[0]/width)*2-1,(topLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_topRight((topRight[0]/width)*2-1,(topRight[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomLeft((bottomLeft[0]/width)*2-1,(bottomLeft[1]/height)*2-1,0,1);
    Eigen::Vector4f canonical_bottomRight((bottomRight[0]/width)*2-1,(bottomRight[1]/height)*2-1,0,1);
    /*Eigen::Vector4f world_topLeft = view.inverse()*canonical_topLeft;
    Eigen::Vector4f world_topRight = view.inverse()*canonical_topRight;
    Eigen::Vector4f world_bottomLeft = view.inverse()*canonical_bottomLeft;
    Eigen::Vector4f world_bottomRight = view.inverse()*canonical_bottomRight;*/

    top_border_vertex.col(0) = canonical_topLeft;
    top_border_vertex.col(1) = canonical_bottomRight;
    top_border_vertex.col(2) = canonical_bottomLeft;
    top_border_vertex.col(3) = canonical_topLeft;
    top_border_vertex.col(4) = canonical_bottomRight;
    top_border_vertex.col(5) = canonical_topRight;
}

void create_bricks(int width, int height) {
    for (int j = height - border_width - 4 * brick_height; j > height - border_width - brick_height * 12; j = j - brick_height) {
        for (int i = border_width + brick_width; i < border_width + 9 * brick_width; i = i + brick_width) {
            Eigen::Vector4f topLeft(i, j, 0, 1);
            Eigen::Vector4f topRight(i + brick_width, j, 0, 1);
            Eigen::Vector4f bottomLeft(i, j - brick_height, 0, 1);
            Eigen::Vector4f bottomRight(i + brick_width, j - brick_height, 0, 1);
            Eigen::Vector4f canonical_topLeft((topLeft[0]/width)*2-1,(topLeft[1]/height)*2-1,0,1);
            Eigen::Vector4f canonical_topRight((topRight[0]/width)*2-1,(topRight[1]/height)*2-1,0,1);
            Eigen::Vector4f canonical_bottomLeft((bottomLeft[0]/width)*2-1,(bottomLeft[1]/height)*2-1,0,1);
            Eigen::Vector4f canonical_bottomRight((bottomRight[0]/width)*2-1,(bottomRight[1]/height)*2-1,0,1);
             
            /*Eigen::Vector4f world_topLeft = view.inverse()*canonical_topLeft;
            Eigen::Vector4f world_topRight = view.inverse()*canonical_topRight;
            Eigen::Vector4f world_bottomLeft = view.inverse()*canonical_bottomLeft;
            Eigen::Vector4f world_bottomRight = view.inverse()*canonical_bottomRight;*/
            
            Eigen::MatrixXf brick_vertex(4, 6);
            brick_vertex.col(0) = canonical_topLeft;
            brick_vertex.col(1) = canonical_bottomRight;
            brick_vertex.col(2) = canonical_bottomLeft;
            brick_vertex.col(3) = canonical_topLeft;
            brick_vertex.col(4) = canonical_bottomRight;
            brick_vertex.col(5) = canonical_topRight;
            
            brick_Vertex_Array.push_back(brick_vertex);

            //put brick vertices into VBO
            VertexBufferObject brick_vertex_VBO;
            brick_vertex_VBO.init();
            brick_vertex_VBO.update(brick_vertex);
            brick_vertex_VBO_array.push_back(brick_vertex_VBO);
        }
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    Eigen::Vector4f p_screen(xpos,height-1-ypos,0,1);
    Eigen::Vector4f p_canonical((p_screen[0]/width)*2-1,(p_screen[1]/height)*2-1,0,1);
    Eigen::Vector4f p_world = view.inverse()*p_canonical;

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        V.col(0) << p_world[0], p_world[1];
        std::cout<<V.col(0)<<std::endl;
        std::cout<<xpos<< " "<<height-1-ypos<<std::endl;
    }
    // Upload the change to the GPU
    VBO.update(V);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Update the position of the first vertex if the keys 1,2, or 3 are pressed
    switch (key)
    {
        case  GLFW_KEY_1:
            V.col(0) << -0.5,  0.5;
            break;
        case GLFW_KEY_2:
            V.col(0) << 0,  0.5;
            break;
        case  GLFW_KEY_3:
            V.col(0) << 0.5,  0.5;
            break;

        case GLFW_KEY_A:
            if(GLFW_PRESS == action) {
                if ((paddle_translate * paddle_vertex.col(0))[0] > -1) {
                    move_multiplier--;
                }
            }
            break;
        case GLFW_KEY_D:
            if(GLFW_PRESS == action) {
                if ((paddle_translate * paddle_vertex.col(5))[0] < 1) {
                    move_multiplier++;
                }
            }
        case GLFW_KEY_G:
            if(GLFW_PRESS == action) {
                game_start = true;
            }
        default:
            break;
    }

    // Upload the change to the GPU
    VBO.update(V);
}

int main(void)
{
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(476, 558, "Arkanoid", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    #ifndef __APPLE__
      glewExperimental = true;
      GLenum err = glewInit();
      if(GLEW_OK != err)
      {
        /* Problem: glewInit failed, something is seriously wrong. */
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      }
      glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
      fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    #endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    VBO.init();

    V.resize(2,3);
    V << 0,  0.5, -0.5, 0.5, -0.5, -0.5;
    VBO.update(V);

    Eigen::MatrixXf brick_texture_coor(2, 6);
    brick_texture_coor.col(0) << 0.0f, 1.0f;
    brick_texture_coor.col(1) << 1.0f, 0.0f;
    brick_texture_coor.col(2) << 0.0f, 0.0f;
    brick_texture_coor.col(3) << 0.0f, 1.0f;
    brick_texture_coor.col(4) << 1.0f, 0.0f;
    brick_texture_coor.col(5) << 1.0f, 1.0f;

    //put brick vertices texture coordinates into VBO
    VertexBufferObject brick_texture_coor_VBO;
    brick_texture_coor_VBO.init();
    brick_texture_coor_VBO.update(brick_texture_coor);


    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float aspect_ratio = float(height)/float(width); // corresponds to the necessary width scaling

    view <<
    aspect_ratio,0, 0, 0,
    0,           1, 0, 0,
    0,           0, 1, 0,
    0,           0, 0, 1;

    grid_topLeft << border_width, height - border_width,0,1;
    grid_topLeft_canonical << (grid_topLeft[0]/width)*2-1,(grid_topLeft[1]/height)*2-1,0,1;

    brick_width_canonical = brick_width / width * 2;
    brick_height_canonical = brick_height / height * 2;
    paddle_width_canonical = brick_width / width * 4;
    paddle_height_canonical = paddle_height / height * 2;
    top_border_width_canonical = border_width / height * 2;
    side_border_width_canonical = border_width / width * 2;
   
    //speed
    speed << 0.5, 0.5;
    speed = speed.normalized() * speed_magnitude;

    create_background(width, height);
    create_ball(width, height);
    create_bricks(width, height);
    create_game_over(width, height);
    create_paddle(width, height);
    create_top_border(width, height);
    create_left_border(width, height);
    create_right_border(width, height);
    create_ready(width, height);

    create_hashmap();

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar* vertex_shader =
            "#version 330 core\n"
                    "in vec4 position;"
                    "in vec2 textureCoor;"
                    "uniform mat4 translate;"
                    "out vec2 TexCoord;"
                    "void main()"
                    "{"
                    "    gl_Position = translate * position;"
                    "    TexCoord = vec2(textureCoor.x, 1.0 - textureCoor.y);"
                    "}";
    const GLchar* fragment_shader =
            "#version 330 core\n"
                    "out vec4 outColor;"
                    "in vec2 TexCoord;"
                    "uniform vec3 triangleColor;"
                    "uniform sampler2D ourTexture;"
                    "void main()"
                    "{"
                    "    outColor = texture(ourTexture, TexCoord);"
                    "}";

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader,fragment_shader,"outColor");
    program.bind();

    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader

    // Save the current time --- it will be used to dynamically change the triangle color
    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_pre = t_start;

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    unsigned int texture0, texture1, texture2, texture3, texture4, texture5, texture6, texture7, texture8, texture9;
    create_texture(GL_TEXTURE0, texture0, "/Users/edkoma/assignment-4-edkoma/red_work.jpg");
    create_texture(GL_TEXTURE1, texture1, "/Users/edkoma/assignment-4-edkoma/orange_work.jpg");
    create_texture(GL_TEXTURE2, texture2, "/Users/edkoma/assignment-4-edkoma/green_work.jpg");
    create_texture(GL_TEXTURE3, texture3, "/Users/edkoma/assignment-4-edkoma/blue_work.jpg");
    create_texture(GL_TEXTURE4, texture4, "/Users/edkoma/assignment-4-edkoma/paddle.jpg");
    create_texture(GL_TEXTURE5, texture5, "/Users/edkoma/assignment-4-edkoma/ball.jpg");
    create_texture(GL_TEXTURE6, texture6, "/Users/edkoma/assignment-4-edkoma/background.jpg");
    create_texture(GL_TEXTURE7, texture7, "/Users/edkoma/assignment-4-edkoma/ready.jpg");
    create_texture(GL_TEXTURE8, texture8, "/Users/edkoma/assignment-4-edkoma/mission_complete.jpg");
    create_texture(GL_TEXTURE9, texture9, "/Users/edkoma/assignment-4-edkoma/mission_failed.jpg");

    identity << 
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    ball_translate << 
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    degree = -10.0;
    Sine = sin(degree * M_PI /180);
    Cosine = cos(degree * M_PI /180);
    rotate_matrix_neg_10 << 
    Cosine, -Sine, Sine, Cosine;

    degree = 10.0;
    Sine = sin(degree * M_PI /180);
    Cosine = cos(degree * M_PI /180);
    rotate_matrix_pos_10 << 
    Cosine, -Sine, Sine, Cosine;


    cout << 2.3 / 2 << endl;
    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Bind your VAO (not necessary if you have only one)
        VAO.bind();

        // Bind your program
        program.bind();

        // move the ball
        auto t_now = std::chrono::high_resolution_clock::now();
        float time_differnce = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_pre).count();
        t_pre = t_now;

        if (game_start == true) {
            ball_update <<
            1, 0, 0, speed[0] * time_differnce,
            0, 1, 0, speed[1] * time_differnce,
            0, 0, 1, 0,
            0, 0, 0, 1;

            ball_translate = ball_update * ball_translate;
            get_grid_number((ball_translate * ball_vertex.col(2))[0], (ball_translate * ball_vertex.col(2))[1], (ball_translate * ball_vertex.col(5))[0], (ball_translate * ball_vertex.col(5))[1]);
            
            //check collision
            for (int i = 0; i < ball_grid.size(); i++) {
                int grid = ball_grid.at(i);
                if (map.find(grid) != map.end()) {
                    int brick_index = map[grid];
                    cout << "hit brick:!!!!!!!!!!!!!!!!!!!!!!!!! " << brick_index << endl;
                    collide(brick_index);
                    map.erase(grid);
                    i = ball_grid.size();
                    break;
                }
            }
            //check left border
            if ((ball_translate * ball_vertex.col(0))[0] <= left_border_vertex.col(5)[0]) {

                speed << -1 * speed[0], speed[1];
            }

            //check right border
            if ((ball_translate * ball_vertex.col(5))[0] >= right_border_vertex.col(0)[0]) {

                speed << -1 * speed[0], speed[1];
            }

            //check top border
            if ((ball_translate * ball_vertex.col(5))[1] >= top_border_vertex.col(2)[1]) {

                speed << speed[0], -1 * speed[1];
            }

            //check paddle
            Eigen::Vector4f a = ball_translate * ball_vertex.col(2);
            Eigen::Vector4f b = paddle_translate * paddle_vertex.col(0);
            Eigen::Vector4f c = ball_translate * ball_vertex.col(1);
            float ball_center_x = (a[0] + c[0]) / 2;

            if (a[1] <= b[1] && 
                (ball_center_x < (paddle_translate * paddle_vertex.col(1))[0] &&
                ball_center_x > b[0])) {
                if (ball_center_x - b[0] < paddle_width_canonical / 4) {
                    speed << speed[0], -1 * speed[1];
                    speed = rotate_matrix_pos_10 * speed;
                } else if (ball_center_x - b[0] > paddle_width_canonical / 4 * 3) {
                    speed << speed[0], -1 * speed[1];
                    speed = rotate_matrix_neg_10 * speed;
                } else {
                    speed << speed[0], -1 * speed[1];
                }
            }

            // Get size of the window
            int width, height;
            glfwGetWindowSize(window, &width, &height);

            paddle_translate << 
            1, 0, 0, move_multiplier * 0.1,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1;
            glUniformMatrix4fv(program.uniform("view"), 1, GL_FALSE, view.data());
        }

        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //draw background
        glUniform1i(program.uniform("ourTexture"), 6);
        glUniformMatrix4fv(program.uniform("translate"), 1, GL_FALSE, identity.data());
        program.bindVertexAttribArray("textureCoor",brick_texture_coor_VBO);
        program.bindVertexAttribArray("position",background_vertex_VBO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        if (deleted_brick_index.size() == 64) {
            game_over = true;
            win = true;
        }
        if(out_of_bound()) {
            game_over = true;
        }

        if (game_start == false) {
            //draw ready
            glUniform1i(program.uniform("ourTexture"), 7);
            glUniformMatrix4fv(program.uniform("translate"), 1, GL_FALSE, identity.data());
            program.bindVertexAttribArray("textureCoor",brick_texture_coor_VBO);
            program.bindVertexAttribArray("position",ready_vertex_VBO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        } else if (game_over == false) {
            // Draw a bricks
            for (int i = 0; i < brick_vertex_VBO_array.size(); i++) {
                if(find(deleted_brick_index.begin(), deleted_brick_index.end(), i) != deleted_brick_index.end()) {
                   continue;
                }
                if (i / 16 == 0) {
                    glUniform1i(program.uniform("ourTexture"), 0);
                } else if (i / 16 == 1) {
                    glUniform1i(program.uniform("ourTexture"), 1);
                } else if (i / 16 == 2) {
                    glUniform1i(program.uniform("ourTexture"), 2);
                } else {
                    glUniform1i(program.uniform("ourTexture"), 3);
                }
                glUniformMatrix4fv(program.uniform("translate"), 1, GL_FALSE, identity.data());
                program.bindVertexAttribArray("textureCoor",brick_texture_coor_VBO);
                program.bindVertexAttribArray("position",brick_vertex_VBO_array.at(i));
            	glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            //draw paddle
            glUniform1i(program.uniform("ourTexture"), 4);
            glUniformMatrix4fv(program.uniform("translate"), 1, GL_FALSE, paddle_translate.data());
            program.bindVertexAttribArray("textureCoor",brick_texture_coor_VBO);
            program.bindVertexAttribArray("position",paddle_vertex_VBO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            //draw ball
            glUniformMatrix4fv(program.uniform("translate"), 1, GL_FALSE, ball_translate.data());
            glUniform1i(program.uniform("ourTexture"), 5);
            program.bindVertexAttribArray("textureCoor",brick_texture_coor_VBO);
            program.bindVertexAttribArray("position",ball_vertex_VBO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        } else {
            //draw game over
            if (win) {
                glUniform1i(program.uniform("ourTexture"), 8);
            } else {
                 glUniform1i(program.uniform("ourTexture"), 9);
            }
            glUniformMatrix4fv(program.uniform("translate"), 1, GL_FALSE, identity.data());
            program.bindVertexAttribArray("textureCoor",brick_texture_coor_VBO);
            program.bindVertexAttribArray("position",game_over_vertex_VBO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }


        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    VAO.free();
    VBO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
