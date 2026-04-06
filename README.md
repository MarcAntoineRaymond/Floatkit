# Floatkit

**Floatkit** is a lightweight, extensible C++ application that lets you display and animate 2D sprites **over your desktop and windows** — like a floating character or animated overlay that lives on your screen.

Designed for playful desktop companions, animated mascots, or interactive overlays, Floatkit provides a flexible foundation for screen-top sprites that can move, respond, and evolve.

---

## ✨ Features

- 🖼️ **2D Sprite Rendering** — Load static or animated images or GIFs (need to provide a cfg file describing the sprites, names, counts, see below for format explanation, or just select a png or gif file).
- 🧲 **Window-Top Overlay** — Renders always on top of other windows while allowing interaction (Does not work with Full Screen Exclusive apps yet, switch to borderless in this case for now).
- 🖱️ **Draggable Sprites** — Drag and place sprites anywhere on your screen.
- 🔍 **Scaling Support** — Resize sprites to fit your visual style or resolution needs (Use mouse wheel).

---

## 🚀 Getting Started

### Build Instructions

No Make file for now, I use visual studio to build it, so you can open the solution file and build it there.

### Load an image

The application add a notify icon that shows a menu when clicking on the icon.

With the Options button you can load a custom cfg file defining sprites names, animation speed and other configuration.

Look at the sample cfg file and sprites in the repository to see how to define your sprites.

## Example Configuration File

```cfg
idle=cat
idle_count=4
idle_fps=2.5
dragging=cat_dragging
dragging_count=10
dragging_fps=5.0
click=cat_jumping
click_count=13
click_fps=10
scale_min=5.0f
scale_max=100.0f
scale_step=5.0f
```

Some parameters are optional, see explanation and defaults value:

|Paramater|Default value|Description|
|----|----|---------|
|idle|idle|Name of the idle sprites, name have the format `<idle>_{n}.png` see samples in assets folder or load GIF if `<idle>.gif` file is present|
|idle_count|0|Number of frames in the idle animation, if not defined the app will crash, ignored if loading gif|
|idle_fps|2.5|Frames per second for the idle animation, ignored if loading gif|
|dragging|dragging|Name of the dragging sprites, name have the format `<dragging>_{n}.png` see samples in assets folder or load GIF if `<click>.gif` file is present|
|dragging_count|0|Number of frames in the dragging animation, ignored if loading gif|
|dragging_fps|2.5|Frames per second for the idle animation, ignored if loading gif|
|click|click|Name of the click sprites, name have the format `<click>_{n}.png` see samples in assets folder or load GIF if `<click>.gif` file is present|
|click_count|0|Number of frames in the click animation, ignored if loading gif|
|click_fps|2.5|Frames per second for the click animation, ignored if loading gif|
|moving|moving|Name of the moving sprites, name have the format `<moving>_{n}.png` see samples in assets folder or load GIF if `<moving>.gif` file is present|
|moving_count|0|Number of frames in the moving animation, ignored if loading gif|
|moving_fps|2.5|Frames per second for the moving animation, ignored if loading gif|
|moving_enabled|false|Enable animation moving toward mouse default to true if moving images are not empty|
|scale_min|0.1|Minimum scale ratio that can be applied for these sprites|
|scale_max|10|Maximum scale ratio that can be applied for these sprites|
|scale_step|0.1|How much scale ratio will change when use mouse wheel to scale sprites|
