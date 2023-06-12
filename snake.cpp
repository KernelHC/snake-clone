//
// Created by KernelHC on 6/3/2023.
//
#include "SFML/Graphics.hpp"
#include <map>
#include <chrono>
#include <thread>
#include <random>
#include <string>

#define MID_X ((float)w->getSize().x/2)
#define MID_Y ((float)w->getSize().y/2)


namespace sg {
    class SnakeNode;
    class Snake;
    void run();
    void update();
    void render();
    void endGame();
    void freeResources();
    typedef enum Direction{UP, DOWN, LEFT, RIGHT} Direction;

    sf::Font *f = new sf::Font();
    sf::RenderWindow *w = new sf::RenderWindow(sf::VideoMode(1000, 800), "Graph Visualizer", sf::Style::Close);
    sf::Event *e = new sf::Event();
    sf::RectangleShape *bite = new sf::RectangleShape(sf::Vector2f(20,20));
    bool new_bite = true;


    struct Score {
        int score = 0;
        sf::Text text;
        void render() {
            text.setString(std::to_string(this->score));
            text.setFont(*f);
            text.setFillColor(sf::Color::White);
            text.setPosition(60,30);
            text.setCharacterSize(40);
            w->draw(text);
        }

        void inc() {
            score++;
        }
    };

    Score* score = new Score();

    struct SnakeNode {
        sf::RectangleShape shape;
        SnakeNode *next = nullptr;
        SnakeNode(int x_pos, int y_pos) {
            this->shape = sf::RectangleShape(sf::Vector2f(18,18));
            this->shape.setFillColor(sf::Color::Green);
            this->shape.setOutlineThickness(2);
            this->shape.setOutlineColor(sf::Color::Black);
            this->shape.setPosition((float)x_pos, (float)y_pos);
        }

        sf::Vector2f getPosition() const {
            return this->shape.getPosition();
        }

        void setPosition(sf::Vector2f pos) {
            this->shape.setPosition(pos);
        }

    };


    struct Snake {
        size_t size = 3;
        SnakeNode *head;
        SnakeNode *tail;

        Direction direction = UP;
        Snake() {
            this->head = new SnakeNode(MID_X, MID_Y);
            head->next = new SnakeNode(MID_X, MID_Y+20);
            head->next->next = new SnakeNode(MID_X, MID_Y+40);
            this->tail = head->next->next;
        }

        ~Snake() {
            SnakeNode *to_delete = this->head, *next_node = this->head->next;
            while (next_node) {
                delete to_delete;
                to_delete = next_node;
                next_node = next_node->next;
            }
            delete next_node;
        }

        [[nodiscard]] bool collide() const {
            SnakeNode* ptr = this->head->next;
            sf::Rect<float> head_bounds = this->head->shape.getGlobalBounds();
            sf::Vector2f head_position = this->head->getPosition();
            while (ptr) {
                if (head_bounds.contains(ptr->getPosition()) || head_position.x < 0 || head_position.x > 980
                        || head_position.y < 0 || head_position.y > 780)
                    return true;
                ptr = ptr->next;
            }
            return false;
        }

        void render() const {
            SnakeNode* to_render = this->head;
            while (to_render) {
                w->draw(to_render->shape);
                to_render = to_render->next;
            }
        }

        void move() {
            if (bite->getGlobalBounds().contains(this->head->getPosition())) {
                this->tail->next = new SnakeNode(0,0);
                this->tail = this->tail->next;
                new_bite = true;
                this->size++;
                score->inc();
            }

            if (this->collide()) {
                endGame();
                this->head = new SnakeNode(MID_X, MID_Y);
                head->next = new SnakeNode(MID_X, MID_Y+20);
                head->next->next = new SnakeNode(MID_X, MID_Y+40);
                this->tail = head->next->next;
                this->size = 3;
            }

            std::map<Direction, sf::Vector2f> dir = {
                    {UP, {0,-20}},
                    {DOWN, {0,20}},
                    {LEFT, {-20,0}},
                    {RIGHT, {20,0}}
            };

            SnakeNode* to_move = this->head;
            sf::Vector2f trail_pos = to_move->getPosition();
            sf::Vector2f curr_pos;
            to_move->setPosition(trail_pos + dir[this->direction]);
            to_move = to_move->next;

            while(to_move) {
                curr_pos = to_move->getPosition();
                to_move->setPosition(trail_pos);
                trail_pos = curr_pos;
                to_move = to_move->next;
            }
        }
    };


    Snake* s = new Snake();



    void run() {
        // Events before being polled are of type Closed by default, to avoid closing the window
        // upon first loss when no other event occurred we use this:
        e->type = sf::Event::MouseButtonPressed;

        f->loadFromFile("Lato-Black.ttf");
        sf::Text game_over;
        game_over.setString("   Press any key to start");
        game_over.setFont(*f);
        game_over.setPosition(MID_X-380, MID_Y-100);
        game_over.setCharacterSize(70);
        w->clear();
        w->draw(game_over);
        w->display();
        bool start = false;
        while (!start) {
            w->pollEvent(*e);
            if (e->type == sf::Event::KeyPressed) start = true;
            if (e->type == sf::Event::Closed) {
                freeResources();
                exit(0);
            }
        }
        bite->setFillColor(sf::Color::Red);
        while (w->isOpen()) {
            update();
            render();
        }
        freeResources();
    }

    void update() {
        std::this_thread::sleep_for(std::chrono::milliseconds(40));

        if (new_bite) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> x_distribution(0, 1000);
            std::uniform_int_distribution<> y_distribution(0, 800);
            int pos_x = x_distribution(gen), pos_y = y_distribution(gen);
            pos_x -= pos_x%20;
            pos_y -= pos_y%20;
            bite->setPosition((float) pos_x, (float) pos_y);
            new_bite = false;
        }
        s->move();

        while (w->pollEvent(*e)) {
            switch (e->type) {
                case sf::Event::Closed: {
                    w->close();
                    break;
                }
                case sf::Event::Resized: {
                    sf::FloatRect view_rect(0, 0, (float) e->size.width, (float) e->size.height);
                    w->setView(sf::View(view_rect));
                    break;
                }
                default: {
                    break;
                }
            }

            if (e->type != sf::Event::KeyPressed) continue;

            switch (e->key.code) {
                case sf::Keyboard::Up:
                case sf::Keyboard::W: {
                    if (s->direction != DOWN && s->head->getPosition().y <= s->head->next->getPosition().y)
                        s->direction = UP;
                    break;
                }
                case sf::Keyboard::Down:
                case sf::Keyboard::S: {
                    if (s->direction != UP && s->head->getPosition().y >= s->head->next->getPosition().y)
                        s->direction = DOWN;
                    break;
                }
                case sf::Keyboard::Left:
                case sf::Keyboard::A: {
                    if (s->direction != RIGHT && s->head->getPosition().x <= s->head->next->getPosition().x)
                        s->direction = LEFT;
                    break;
                }
                case sf::Keyboard::Right:
                case sf::Keyboard::D: {
                    if (s->direction != LEFT && s->head->getPosition().x >= s->head->next->getPosition().x) s->direction = RIGHT;
                    break;
                }
                default: break;
            }
        }
    }

    void render() {
        w->clear();
        s->render();
        w->draw(*bite);
        score->render();
        w->display();
    }

    void endGame() {
        sf::Text game_over;
        game_over.setString("    How are you this bad?!?!\n\n      press space to restart!");
        game_over.setFont(*f);
        game_over.setPosition(MID_X-450, MID_Y-150);
        game_over.setCharacterSize(70);
        w->clear();
        w->draw(game_over);
        w->display();
        bool restart = false;
        while (!restart) {
            w->pollEvent(*e);
            if (e->type == sf::Event::KeyPressed && e->key.code == sf::Keyboard::Space) restart = true;
            if (e->type == sf::Event::Closed) {
                freeResources();
                exit(0);
            }
        }
        delete score;
        score = new Score();
        s->direction = UP;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> x_distribution(0, 1000);
        std::uniform_int_distribution<> y_distribution(0, 800);
        int pos_x = x_distribution(gen), pos_y = y_distribution(gen);
        pos_x -= pos_x%20;
        pos_y -= pos_y%20;
        bite->setPosition((float) pos_x, (float) pos_y);
        new_bite = false;
    }

    void freeResources() {
        delete s;
        delete bite;
        delete f;
        delete e;
        delete w;
        delete score;
    }
}
using namespace sg;

int main() {
    run();
    return 0;
}