import pygame as pg
from tkinter import Tk
from tkinter.simpledialog import askstring
from tkinter.messagebox import askyesno
from tkinter.filedialog import askopenfilename
import time
import sys

sys.setrecursionlimit(5000)

# to select blocks use the number keys
# to place click (and drag)
# to save press the 'o' key
# see other comments for info on how to load and save levels

def get_mouse_coords(event):
    pos = event.dict["pos"]
    return (int(pos[0]/16), int(pos[1]/16))

def place_block(level, pos, block):
    level[pos[1]][pos[0]] = block

def load_textures(file):
    tile_data = open(file)
    num_tiles = int(tile_data.readline().strip())
    out = []
    for i in range(num_tiles):
        line = tile_data.readline()
        out += [pg.image.load("resources/textures/blocks/{}".format(line.split(" ")[0]))]
    return out

def level_to_text_file(level_array, level_name, tileset_name):
    if not askyesno("Castle Creator", "Do you want to save this file to \n{}?".format(level_name)):
        return False
    file = open(level_name, "w+")
    file.write(tileset_name + "\n")
    file.write("{} {}\n".format(len(level_array), len(level_array[0])))
    for row in level_array:
        line = ""
        for block in row:
            line += "{} ".format(block)
        file.write(line.strip() + "\n")
    print("Saved level data to {}".format(level_name))
    return True

def level_to_image(window, level_name):
    try:
        pg.image.save(window, level_name)
        print("Saved level preview to {}".format(level_name))
    except Exception:
        print("WARNING: The level preview was not saved")
        
def save_level(window, level_array, save_to, texture_dir):
    if level_to_text_file(level_array, "resources/levels/"+ save_to + ".txt", texture_dir):
        level_to_image(window, "resources/levels/previews/" + save_to + ".png")

def fill_area(level, pos, target, start):
    if pos[0] < 0 or pos[1] < 0 or pos[1] >= len(level) or \
       pos[0] >= len(level[0]) or level[pos[1]][pos[0]] == target or \
       level[pos[1]][pos[0]] != start:
        return
    else:
        level[pos[1]][pos[0]] = target
        fill_area(level, (pos[0] - 1, pos[1]), target, start)
        fill_area(level, (pos[0] + 1, pos[1]), target, start)
        fill_area(level, (pos[0], pos[1] - 1), target, start)
        fill_area(level, (pos[0], pos[1] + 1), target, start)
    return

def mirror(level):
    for i in range(len(level)):
        halfway = int(len(level[i])/2)
        for j in range(halfway):
            level[i][len(level[i])-1-j] = level[i][j]

def init_level(dimensions = None, file = None):
    if file:
        file = open(file,"r")
        texture_file = file.readline().strip()
        sizes = file.readline().split(" ")
        size = (int(sizes[1]),int(sizes[0]))
        level = []
        for line in file.readlines():
            row = []
            for c in line.split(" "):
                row += [int(c)]
            level += [row]
        return (texture_file,size,level)
    else:
        level = []
        for i in range(dimensions[1]):
            row = []
            for j in range(dimensions[0]):
                row += [0]
            level += [row]
        return level
def strip_name(file_name):
    while file_name.find("/") != -1:
        file_name = file_name[file_name.find("/") + 1:len(file_name)]
    return file_name.replace(".txt","")

def main(load_file_name):
    save_to = "levels/my_castle.txt" # CHANGE SAVE DESTINATION HERE
    texture_file = "resources/textures/normal.txt" # SET TEXTURES HERE
    (width, height) = (100, 50) # SET THE LEVEL SIZE HERE
    level_array = None

    if load_file_name == "":
        pass
        save_to = askstring("Castle Creator","What do you want this level to be called?")
        level_array = init_level(dimensions = (width,height)) # init empty
    else:
        save_to = strip_name(load_file_name)
        level = init_level(file = load_file_name)
        texture_file = level[0]
        (width, height) = level[1]
        level_array = level[2]
    

    textures = load_textures(texture_file)
    background_color = (0,0,0) # black
    block_size = 16
    mouse_down = False
    running = True
    selected_block = 0

    screen = pg.display.set_mode((width * block_size, height * block_size))
    pg.display.set_caption('Castle Creator')

    mouse_pos = (0,0)
    scroll = (0,0)
    f_down = False

    while running:
        for event in pg.event.get():
            if event.type == pg.MOUSEMOTION:
                mouse_pos = get_mouse_coords(event)
            if event.type == pg.QUIT:
                running = False
                scroll = (0,0)
                save_level(screen, level_array, save_to, texture_file)
            elif event.type == pg.MOUSEBUTTONDOWN or event.type == pg.MOUSEMOTION and mouse_down:
                mouse_down = True
                if f_down:
                    try:
                        fill_area(level_array, (mouse_pos[0] - scroll[1], mouse_pos[1] - scroll[0]), selected_block, level_array[mouse_pos[1]][mouse_pos[0]])
                    except RecursionError as e:
                        print("The area you tried to fill was too large!")
                    mirror(level_array)
                    continue
                mirror(level_array)
                place_block(level_array, (mouse_pos[0] - scroll[1], mouse_pos[1] - scroll[0]) , selected_block)
            elif event.type == pg.MOUSEBUTTONUP:
                mouse_down = False
            elif event.type == pg.KEYDOWN:
                key = event.dict["key"]
                if key == pg.K_o:
                    scroll = (0,0)
                    save_level(screen, level_array, save_to, texture_file)
                if key == pg.K_f:
                    f_down = True
                if key - 48 >= 0 and key - 48 <10:
                    selected_block = key - 48
            elif event.type == pg.KEYUP:
                key = event.dict["key"]
                if key == pg.K_f:
                    f_down = False
                elif key == pg.K_LEFT:
                    scroll = (scroll[0], scroll[1] + 1)    
                elif key ==pg.K_RIGHT:
                    scroll = (scroll[0], scroll[1] - 1)    
                elif key ==pg.K_DOWN:
                    scroll = (scroll[0] - 1, scroll[1])    
                elif key ==pg.K_UP:
                    scroll = (scroll[0] + 1, scroll[1])        
        screen.fill(background_color)
        for i in range(len(level_array)):
            for j in range(len(level_array[i])):
                screen.blit(textures[level_array[i][j]],(j * block_size + 16 * scroll[1], i * block_size + 16 * scroll[0]))
        
        pg.display.update()
    pg.quit()

if __name__ == "__main__":


    root = Tk()
    root.withdraw()
    root.update()
    filename = askopenfilename()
    main(filename)