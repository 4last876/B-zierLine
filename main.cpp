#include <iostream>
#include <memory>
#include <iterator>
#include <set>
#include <map>
#include <SDL2/SDL.h>
#include <vector>
#include <algorithm>
#include <string>
#include <cmath>

//MODEL

struct figure{
  figure(int x_,int y_) : x(x_),y(y_){}
int x = 0;
int y = 0;

bool operator== (const figure& other){
  return other.x == x && other.y == y;
}
virtual ~figure(){}
};

struct point : public figure{
  point(int x_, int y_) : figure(x_,y_){}
};

struct rect : public point{
  rect(int x_,int y_,int w_,int h_) : point(x_,y_),w(w_),h(h_){}
int w = 0;
int h = 0;
};

struct line : public figure{
  line(point *p1_,point *p2_) : figure{0,0}, p1(p1_),p2(p2_){}
point* p1;
point* p2;

};

struct bezierLine : public figure{
std::vector<point*> points;
bezierLine(point* p1_, point* p2_, point* p3_) : figure{0,0}, points{p1_,p2_,p3_} {}
};

//VIEW
class Isubcride{
  public:
  virtual void update(std::vector<figure*> data) = 0;
  virtual ~Isubcride(){
    
  }
};

class render : public Isubcride{
constexpr static int SCREEN_WIDTH = 500;
constexpr static int SCREEN_HEIGHT = 500;
bool quit = false;

std::shared_ptr<SDL_Window> win;
std::shared_ptr<SDL_Renderer> renderer;

constexpr static void DeleterRend(SDL_Renderer* renderer){
SDL_DestroyRenderer(renderer);
}


constexpr static void DeleterWin(SDL_Window* win){
SDL_DestroyWindow(win);
}
  public:
render(){
  if(SDL_Init(SDL_INIT_EVERYTHING) < 0){
        std::cout << "Error: " << SDL_GetError() << std::endl;
    }
win.reset(SDL_CreateWindow("Love",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,SCREEN_WIDTH,SCREEN_HEIGHT,SDL_WINDOW_SHOWN),DeleterWin);
    if(win.get() == nullptr){
        std::cout << "Error: " << SDL_GetError() << std::endl;
    }

renderer.reset(SDL_CreateRenderer(win.get(),-1,SDL_RENDERER_ACCELERATED),DeleterRend);

    if(renderer.get() == nullptr){
        std::cout << "Error: " << SDL_GetError() << std::endl;
    } 
}

  void update(std::vector<figure*> data) override{
  SDL_SetRenderDrawColor(renderer.get(),255,255,255,255);
  SDL_RenderClear(renderer.get());

     for(auto au : data){
      if(line* l = dynamic_cast<line*>(au)){
        SDL_SetRenderDrawColor(renderer.get(),0,0,0,255);
        SDL_RenderDrawLine(renderer.get(),l->p1->x,l->p1->y,l->p2->x,l->p2->y);
      }
      if(rect* r = dynamic_cast<rect*>(au)){
        SDL_Rect r_{r->x,r->y,r->w,r->h};
        SDL_SetRenderDrawColor(renderer.get(),0,0,0,255);
          SDL_RenderDrawRect(renderer.get(),&r_);
        }
        
      if(point* r = dynamic_cast<point*>(au)){
        SDL_RenderDrawPoint(renderer.get(),r->x,r->y);
        }

      if(bezierLine* r = dynamic_cast<bezierLine*>(au)){
           SDL_SetRenderDrawColor(renderer.get(),0,0,0,255);
           SDL_RenderDrawLine(renderer.get(),r->points[0]->x,r->points[0]->y,r->points[1]->x,r->points[1]->y);
           SDL_RenderDrawLine(renderer.get(),r->points[1]->x,r->points[1]->y,r->points[2]->x,r->points[2]->y);
        
        int last_x = r->points[0]->x;
        int last_y = r->points[0]->y;
           for(float t = 0; t < 1; t += 0.01){
            int x = pow((1 - t),2) * r->points[0]->x;
            int y = pow((1 - t),2) * r->points[0]->y;

            x += (2 * t) * (1 - t) * r->points[1]->x;
            y += (2 * t) * (1 - t) * r->points[1]->y;

            x += pow(t,2) * r->points[2]->x;
            y += pow(t,2) * r->points[2]->y;

            SDL_RenderDrawLine(renderer.get(),last_x,last_y,x,y);

            last_x = x;
            last_y = y;
           }
        }

     }

  SDL_RenderPresent(renderer.get());
  }

  ~render(){

}
};


class updateFigure : public Isubcride {
    std::vector<SDL_Event> events;
    bool dragging = false;
    rect* selected = nullptr;
    int offsetX = 0, offsetY = 0;

public:
    void addEvent(SDL_Event& e) {
        events.push_back(e);
    }

    void update(std::vector<figure*> data) {
        for (auto& e : events) {

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int x = e.button.x;
                int y = e.button.y;

                for (auto au : data) {
                    if (rect* r = dynamic_cast<rect*>(au)) {
                        if (x >= r->x && x < (r->x + r->w) &&
                            y >= r->y && y < (r->y + r->h)) {
                            dragging = true;
                            selected = r;

                            offsetX = x - r->x;
                            offsetY = y - r->y;
                            break;
                        }
                    }
                }
            }

            else if (e.type == SDL_MOUSEMOTION) {
                if (dragging && selected != nullptr) {
                    selected->x = e.motion.x - offsetX;
                    selected->y = e.motion.y - offsetY;
                }
            }

            else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                dragging = false;
                selected = nullptr;
            }
        }

        events.clear();
    }
};



//CONTROLLER
class ModelManager{

std::map<int,Isubcride*> subcrides;
  public:
  void subcride(Isubcride* sub,int key){
    subcrides.emplace(key,sub);
  }

  void unsubcride(int key){
      subcrides.erase(key);
  }

  void notify(std::vector<figure*> data){
    for(auto& au : subcrides){
      auto[key,sub] = au;
        sub->update(data);
    }
  }
};

class Model{
std::vector<figure*> data;
public:
ModelManager manager;
void addData(figure* fig){
data.push_back(fig);
manager.notify(data);
}

void removeData(figure* fig){
auto it = std::find(std::begin(data),std::end(data),fig);
if(it != std::end(data)){
data.erase(it);
manager.notify(data);
}
}

std::vector<figure*>& getData(){
  return data;
}

void update(){
  manager.notify(data);
}

int size(){
  return data.size();
}

};

namespace std{
ostream&  operator<<(std::ostream& os, const figure& fig){
return os << '[' << fig.x << ' ' << fig.y << ']' << std::flush;
  }
}

class ApplicationLayer{
render rend;
Model model;
updateFigure updatefigure;
bool is_open = true;

void updater(){
SDL_Event e;
   while(SDL_PollEvent(&e) != 0){
    updatefigure.addEvent(e);
        if(e.type == SDL_QUIT){
            is_open = !is_open;
        }
   }
}
public:

ApplicationLayer(){
model.manager.subcride(&rend,1);
model.manager.subcride(&updatefigure,2);
}

void run(){
  rect rt1(100,100,10,10);
  rect rt2(100,300,10,10);
  rect rt3(100,400,10,10);
bezierLine li{&rt1,&rt2,&rt3};
model.addData(&rt1);
model.addData(&rt2);
model.addData(&rt3);
model.addData(&li);
  while(is_open){
updater();
model.update();
   }
}
};


int main(){
ApplicationLayer app;
app.run();
}
