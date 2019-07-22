import contextlib
from os import listdir
with contextlib.redirect_stdout(None):
    import pygame as pg
import socket as sock
import threading
import subprocess
from time import sleep

# This exists


stage ="host/connect/local"
ip_string = "CONNECTING"
button_click_sound = None
time_left = None
delay = 4000
GIF_FPS = .23

class game_launcher():
    def __init__(self, game_file):
        self.file = game_file
        self.args = []

    def set_args(self, args):
        self.args = [self.file] + args
            
    def launch_game(self):
        try:
            if(self.args[2] == "42069"):
                return
        except Exception:
            pass
        try:
            t = threading.Thread(target=delay_exit, args=[self.args])
            t.daemon = True
            t.start()
            exit()
        except FileNotFoundError as e:
            print("Could not find the game to launch. Please make sure you have not rearranged any of the directories.\nAlso make sure you are running the launcher with ./launcher/start")
            exit()

def delay_exit(args):
    subprocess.call(args)
    exit()
    
castle_launcher = game_launcher("./bin/castlebreaker")

def get_IP(): 
    global ip_string

    try: 
        s = sock.socket(sock.AF_INET, sock.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip_string = s.getsockname()[0]
        s.close()
    except: 
        ip_string = "Unable to get IP"
    exit()

def blit_alpha(target, source, location, opacity):
        x = location[0]
        y = location[1]
        temp = pg.Surface((source.get_width(), source.get_height())).convert()
        temp.blit(target, (-x, -y))
        temp.blit(source, (0, 0))
        temp.set_alpha(opacity)        
        target.blit(temp, location)

class animation():
    def __init__(self, dir, amount, extra = 0, start = 0, transform = None):
        self.amount = amount
        self.frames = []
        self.current = 0
        for i in range(amount):
            try:
                self.frames += [pg.image.load("{}{}.png".format(dir, i))]
            except Exception:
                self.frames += [pg.image.load("{}{}{}.png".format(dir, "0" * ((len(str(amount)) + extra) - len(str(start + i))), start + i))]
            if transform:
                self.frames[i] = pg.transform.scale(self.frames[i], transform, )
    
    def draw(self, surface, pos, speed):
        surface.blit(self.frames[int(self.current)], pos)
        self.current += speed
        self.current %= self.amount

class LevelSelector():
    def __init__(self, dir, box):
        self.levels = []
        self.level_names = []
        self.current = 0
        self.box = box
        for file in listdir(dir):
            if ".png" not in file:
                continue
            self.levels += [pg.transform.scale(pg.image.load(dir + file), (box.w, box.h))]
            self.level_names += [dir + file]
        pass

    def change_level(self, amount):
        self.current += amount
        self.current %= len(self.levels)

    def draw(self, screen):
        screen.blit(self.levels[self.current], self.box)

    def update(self, event):
        if event.type == pg.MOUSEBUTTONDOWN:
            if self.box.collidepoint(event.pos):
                if event.pos[0] > self.box.w/2 + self.box.x:
                    self.change_level(1)
                else:
                    self.change_level(-1)

    def get_level_name(self):
        return self.level_names[self.current]

class writable_text_box():
    def __init__(self, box, colors):
        self.default_width = box.w
        self.font = None
        self.comic_sans = False
        try:
            self.font = pg.font.Font("resources//comic_sans//normal.ttf", 32)
            self.comic_sans = True
        except:
            self.font = pg.font.Font(None, 32)
        self.input_box = box
        self.active = False
        self.color_inactive = pg.Color(colors[0])
        self.color_active = pg.Color(colors[1])
        self.color_back = pg.Color(colors[2])
        self.color = self.color_inactive
        self.color_text = pg.Color(colors[3])
        self.text = ''
    
    def draw(self, screen):
        # Render the current text.
        txt_surface = self.font.render(self.text, True, self.color_text)
        # Resize the box if the text is too long.
        self.input_box.w = max(self.default_width, txt_surface.get_width() + 10)

        back = pg.Surface((self.input_box.w, self.input_box.h))
        back.fill(self.color_back)
        blit_alpha(screen, back, (self.input_box.x, self.input_box.y),128)

        if self.comic_sans:
            screen.blit(txt_surface, (self.input_box.x+5, self.input_box.y-5))
        else:
            screen.blit(txt_surface, (self.input_box.x+5, self.input_box.y+5))
            
        pg.draw.rect(screen, self.color, self.input_box, 2)
        (self.color_back)

    def update(self, event):
        if event.type == pg.MOUSEBUTTONDOWN:
            if self.input_box.collidepoint(event.pos):
                self.active = not self.active
            else:
                self.active = False
            self.color = self.color_active if self.active else self.color_inactive
        if event.type == pg.KEYDOWN:
            if self.active:
                if event.key == pg.K_RETURN:
                    if self.text == "42069":
                        print("nice ;)")
                elif event.key == pg.K_BACKSPACE:
                    self.text = self.text[:-1]
                else:
                    self.text += event.unicode


class button():
    def __init__(self, box, text, on_click, colors, text_box = False, font_size =32):
        self.def_w = box.w
        self.text_box = text_box
        self.font = None
        self.comic_sans = False
        try:
            self.font = pg.font.Font("resources//comic_sans//normal.ttf", font_size)
            self.comic_sans = True
        except:
            self.font = pg.font.Font(None, font_size)
        self.box = box # pg.Rect(100, 100, 140, 40)
        self.active = False
        self.color_inactive = pg.Color(colors[0])
        self.color_active = pg.Color(colors[1])
        self.color_back = pg.Color(colors[2])
        self.color = self.color_inactive
        self.color_text = pg.Color(colors[3])
        self.text = text
        self.action = on_click
    
    def draw(self, screen):
        # Render the current text.
        txt_surface = self.font.render(self.text, True, self.color_text)
        # Resize the box if the text is too long.
        self.box.w = max(self.def_w, txt_surface.get_width()+20)
        if not self.text_box:
            back = pg.Surface((self.box.w, self.box.h))
            back.fill(self.color_back)
            screen.blit(back, (self.box.x, self.box.y))
        
        off_x = get_edge(self.box.w, txt_surface.get_width())
        off_y = get_edge(self.box.h, txt_surface.get_height())

        if self.comic_sans:
            screen.blit(txt_surface, (self.box.x + off_x, self.box.y + off_y))
        else:
            screen.blit(txt_surface, (self.box.x + off_x, self.box.y + off_y))
        if not self.text_box:
            pg.draw.rect(screen, self.color, self.box, 2) 

    def update(self, event, args = 0):
        if event.type == pg.MOUSEBUTTONDOWN:
            if self.box.collidepoint(event.pos):
                self.active = not self.active
                if self.action:
                    if args != 0:
                        self.action(args)
                    else:
                        self.action()
            else:
                self.active = False
            self.color = self.color_active if self.active else self.color_inactive

def button_click(stage_set):
    global stage, time_left
    button_click_sound.play()
    if stage_set:
        stage = stage_set
        if stage_set == "loading":
            pg.mixer.music.fadeout(5000)
            time_left = 5100

def host_click():
    button_click("host")

def connect_click():
    button_click("connect")

def back_click():
    button_click("host/connect/local")

def local_click():
    button_click("local")
    # TODO load the local version

def go_host_click(level_name):
    button_click("loading")
    level_name = level_name.replace("previews//","").replace(".png",".txt")
    castle_launcher.set_args(["server", "8088", level_name])

def go_connect_click(ip_entered):
    button_click("loading")
    castle_launcher.set_args(["client", ip_entered, "8088"])

def go_local_click(level_name):
    level_name = level_name.replace("previews//","").replace(".png",".txt")
    button_click("loading")
    castle_launcher.set_args(["local", level_name])

def play_music(path):
    pg.mixer.music.stop()
    pg.mixer.music.load(path)
    pg.mixer.music.play(-1)

def get_edge(width, size):
    return int((width - size) / 2)

class loader():
    def __init__(self, de, rate, limit = 4):
        self.i = 0
        self.limit = limit
        self.default = de
        self.rate = rate

    def get_string(self, current):
        if self.default == current:
            out = current + "." * int(self.i)
            self.i += self.rate
            self.i %= self.limit
            return out
        else:
            return current

def main():
    global time_left, delay

    play_music('resources//audio//music//field_of_heroes.mp3')
    (width,height) = (640, 480)
    screen = pg.display.set_mode((width, height))
    pg.display.set_caption('Castle Breaker CDXX LXIX Launcher')
    cbx = get_edge(width, 200)
    
    ip_loader = loader("CONNECTING", 0.05)
    game_loader = loader("LOADING", 0.05)
    levels = LevelSelector("resources//levels//previews//", pg.Rect(get_edge(width, 300), 235, 300, 150))

    playing = False
    clock = pg.time.Clock()
    button_colors = ("black","darkgrey","black","white")
    ip_label = button(pg.Rect(0, 146, width, 40),"HOST IP", None, button_colors, text_box = True)
    ip_host = button(pg.Rect(cbx, 180, 200, 40), ip_string , None, ("black","black","black","white"))
    ip = writable_text_box(pg.Rect(get_edge(width, 200), 250, 200, 40), ("black","darkgrey","black","white"))
    go_host = button(pg.Rect(cbx, 400, 200, 40),"GO", go_host_click, button_colors)
    go_connect = button(pg.Rect(cbx, 400, 200, 40),"GO", go_connect_click, button_colors)
    go_local = button(pg.Rect(cbx, 400, 200, 40),"GO", go_local_click, button_colors)
    host = button(pg.Rect(cbx, 300, 200, 40), "HOST", host_click, button_colors)
    connect = button(pg.Rect(cbx, 350, 200, 40), "CONNECT", connect_click, button_colors)
    local = button(pg.Rect(cbx, 400, 200, 40), "LOCAL", local_click, button_colors)
    back = button(pg.Rect(0, 0, 0, 28), "BACK", back_click, button_colors, font_size = 20)
    done = False
    background = pg.image.load("resources//graphics//launcher.png")
    soon = pg.image.load('resources//graphics//coming_soon_tm.png')
    stock = pg.image.load('resources//graphics//stock.png')

    flipper = animation("resources//graphics//title//", 36, transform = (620,80))
    rick = animation("resources//graphics//rick//", 499, extra = 1, start = 1)
    

    while not done:
        for event in pg.event.get():
            if event.type == pg.QUIT:
                done = True
            if stage == "host/connect/local":
                host.update(event)
                connect.update(event)
                local.update(event)
            elif stage == "connect":
                ip.update(event)
                go_connect.update(event, args = ip.text)
            elif stage == "host":
                go_host.update(event, args = levels.get_level_name())
                levels.update(event)
            elif stage == "local":
                go_local.update(event, args = levels.get_level_name())
                levels.update(event)

            if not stage == "loading" and not stage == "host/connect/local":
                back.update(event)

        screen.blit(background,(0, 0))
        flipper.draw(screen, (10,20), .7)
        if stage == "host/connect/local":
            host.draw(screen)
            connect.draw(screen)
            local.draw(screen)
        elif stage == "connect":
            ip.input_box.x = get_edge(width, ip.input_box.w)
            ip.draw(screen)
            ip_label.text = "HOST IP"
            ip_label.box.y = 216
            ip_label.draw(screen)
            go_connect.draw(screen)
        elif stage == "host":
            levels.draw(screen)
            ip_label.text = "YOUR IP"
            ip_label.box.y = 146
            ip_label.draw(screen)
            ip_host.text = ip_loader.get_string(ip_string)
            if "CONNECTING" in ip_host.text:
                ip_host.def_w = 300
            else:
                ip_host.def_w = 200
            ip_host.box.x = get_edge(width, ip_host.box.w)
            ip_host.draw(screen)
            go_host.draw(screen)
        elif stage == "local":
            go_local.draw(screen)
            levels.draw(screen)
        elif stage == "loading":
            ip_host.text = game_loader.get_string("LOADING")
            ip_host.box.y = 250
            ip_host.box.x = get_edge(width, ip_host.box.w)
            ip_label.box.y = 200
            ip_host.draw(screen)
            pass
        if not stage == "loading" and not stage == "host/connect/local":
            back.draw(screen)

        if time_left:
            time_left -= clock.get_time()
            if time_left <= 0:
                castle_launcher.launch_game()
                pg.display.flip()
                delay -= clock.get_time()

        if delay and delay != 4000:
            screen.blit(soon,(0,0))

            if delay < 0:
                playing = True  
                time_left = None
                delay = None 
                play_music('resources//audio//music//rick.mp3')
                
        if playing:
            screen.blit(soon,(0,0))
            rick.draw(screen,(160,100), GIF_FPS)
            screen.blit(stock,(0, 0)) 

        pg.display.flip()
        clock.tick(60)

if __name__ == '__main__':
    pg.init()
    button_click_sound = pg.mixer.Sound('resources//audio//chunks//Boom.wav')
    t = threading.Thread(target=get_IP)
    t.daemon = True
    t.start()
    main()
    pg.quit()