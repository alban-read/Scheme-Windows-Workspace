## Graphics

 [Index](welcome.html)  

#### <a name="image-pane">The image pane</a>

Graphics commands can be used to draw lines and shapes that can be displayed in the image pane.

There are a couple of examples in the scripts folder; you can run them to do some drawing.  

Also you can try out the examples in the documentation.

#### <a name="graphics-functions">Graphics functions</a>

These functions work on an 'offscreen' image in memory; the image pane is just a way to display them.

You can also create drawings and save them to a file; without showing them in the image pane.

Windows includes a graphics library (GDI+);  that provides these drawing functions.

####  Showing the image pane

- If you have set Docking to Browser Layout;  you can select the image pane to the right.
- If not using the documentation: Set Docking to Image Layout.
- You can drag and drop an example from the scripts folder to the Evaluator view; such as drawtree.ss which will draw a tree shape.
- Or select a function from this browser documentation pane and shift-run it.

#### Warning

- On  high DPI screens; the image pane becomes very large.
-  It helps performance to keep the image pane; a reasonable size; or just use a 1920x1080 screen mode.

 

------



## Graphics

##### A fractal fern

You can select and execute (shift-return) this script to create a fern-like image.

```Scheme
;; fractal fern 
(define x 250.0)
(define y 200.0)
(define x0 0.0)
(define y0 0.0)
;;
(define transform
  (lambda ()
    (set! x0 x)
    (set! y0 y)
    (let ([r (random 100)])
      (cond
        [(< r 1) (begin (set! x 0.0) (set! y (* 0.16 y0)))]
        [(< r 86)
         (begin
           (set! x (+ (* 0.85 x0) (* 0.04 y0)))
           (set! y (+ (* -0.04 x0) (* 0.85 y0) 1.6)))]
        [(< r 93)
         (begin
           (set! x (+ (* 0.2 x0) (* -0.26 y0)))
           (set! y (+ (* 0.23 x0) (* 0.22 y0) 1.6)))]
        [else
         (begin
           (set! x (+ (* -0.15 x0) (* 0.28 y0)))
           (set! y (+ (* 0.26 x0) (* 0.24 y0) 0.44)))]))))
;;
(define draw-point
  (lambda ()
    (rset-pixel (+ (* x 50) 200) (- 500 (- (* y 50) 10)))))
;;
(define init
  (lambda ()
    (colour 0 200 0 255)
    (paper 0 0 0 255)
    (clr 500 500)
    (show)))
;;
(define fern
  (lambda () 
    (dotimes 100 
      (transform) (draw-point)) (show)))
;;
(init)
;; select and shift-return
(fern)
;;
```

 Select the (fern) command and press.

```
<shift><enter>
```

To progressively drawn more of the fern pattern.

Using set-pixel is very slow; and not generally recommended. 

There are commands to draw lines and rectangles that are much faster.

## Graphics 2D commands

- For 2D graphics Windows GDI+ provides smooth anti-aliased lines.
- The app has a pane that displays an image.
- All graphics operations draw onto the active surface which is an **off-screen bitmap**.
- An image is displayed when the **show** function is called.

------

There is a single active surface that all these commands draw on.

------

*Set the docking mode to image layout in the shell.*

#### Create  an image

An image always has a background paper colour or gradient.

Set the paper colour to an r,g,b,a (red,green,blue,alpha) value; then create a blank image

```scheme
;; paper color r,g,b, alpha
(paper 40 140 240 255) 
;; create and clear 640x320 image	
(clr 640 320) 	
;; show on screen
(show) 

```

Or set the gradient brush then create the blank image

```scheme
(gradient 
 1 1 640 320 	;; rectangle
 100 10 0 255 	;; color 1
 10 10 100 255	;; color 2
 0.45		    ;; angle
 #t)
;;
(clrg 640 320)	;; clear with gradient
(show)
```

#### Save the image to a file

```scheme
(save-as-png "myimage.png")
(save-as-jpeg "myimage.jpeg")
```

------

#### Show the image on screen

To re-display a changed image on the image view

```scheme
;; show the image
(show) 
```



## Colours

- Colours are set for pens and for brushes.
- The colours are set using red, green, blue, and alpha values (0..255).
- Alpha is used to blend.
- There is one pen; set using **colour**.
- A solid brush has a colour; there is a paper brush; a fill brush and a gradient brush used by different commands.
- A linear gradient brush has two colours and an angle.
- You set the relevant paper, fill and gradient brushes using the **paper**, **fill** and **gradient** procedures.



 

------

### Drawing  

*Display a drawing with (show)*

- Remember that no changes are displayed on the screen until you show the image
- show swaps the active surface with the display surface; and tells the image pane to update.

The examples assume an image has been created.

#### First create an image

```Scheme
;; paper color r,g,b, alpha
(paper 40 140 240 255)

;; clear 640x320 image	
(clr 640 320) 

;; show it
(show)
```

An image has a paper colour and a size.



### Lines

#### Line drawing

A line has a colour and a pen width

```scheme
(colour 0 200 0 255) ;; green 
(pen-width 2.2)	   ;; wide line	
(draw-line 10 10 200 200) ;; draw a line
(show)
```

#### Shape Drawing

The shape commands can draw an outline fill a shape gradient fill a shape or hatch fill a shape.

The commands are draw-thing, fill-thing, gradient-thing and hatch-thing.

#### Rectangle drawing

```scheme
(colour 200 0 200 255)  ;; purple
(pen-width 3.5)	   ;; wide line	
(draw-rect 10 10 40 40) ;; x,y, width height
(show)
```

#### Ellipse Drawing

```scheme
(colour 0 0 200 255) ;; blue 
(pen-width 0.5) ;; narrow line
(draw-ellipse 80 10 50 50) ;; x,y, width, height
(show)
```

#### Pie Drawing

```scheme
;; set colour
(colour 0 200 200 255) 
;; thick line
(pen-width 8.0)
;; x,y,width,height,start,end
(draw-pie 120 100 150 150 0 100) 
(show)
```

#### Arc Drawing

```scheme
;; set colour
(colour 200 100 200 255) 
(pen-width 4.0)
;; x,y,width,height,start,end
(draw-arc 10 10 150 150 0 100)
(show)
```

### Fill shapes

A filled shape can have a solid brush a gradient brush or a hatch brush.

#### Fill Rectangle

```scheme
;; set solid fill brush color
(fill 100 0 100 255) 
;; x,y,w,h
(fill-rect 100 100 50 50)
(show)
```

#### Hatch fill rectangles

```Scheme
;; lets go for a black background
(paper 0 0 0 255)
;; clear 640x320 image	
(clr 640 320) 

;; hatch brush has a style and two colours
(define yellow '(255 255 85 255))
(define orange '(230 109 25 255))
(define blue '(0 0 200 255))
(define gray '(100 100 100 255))
(define DottedDiamond '(44))
(define HorizontalBrick '(39))
(define Shingle '(45))

;; set the hatch brush style and colours
(apply hatch 
 (append DottedDiamond yellow blue))
;; set the drawing colour
(apply colour yellow)
(pen-width 1.0)

;; rectangle position and size
(define my-rectangle '(10 10 50 50))

;; first sample
(apply hatch-rect my-rectangle )
(apply draw-rect my-rectangle)
(show)

;; try another pattern in a second
(after 1000
 (lambda () 
  (apply hatch 
   (append HorizontalBrick yellow orange))
  (apply hatch-rect my-rectangle )
  (apply draw-rect my-rectangle)
  (show)))

;; try another pattern in two seconds
(after 2000
 (lambda () 
  (apply hatch 
   (append Shingle blue gray))
  (apply hatch-rect my-rectangle )
  (apply draw-rect my-rectangle)
  (show)))


```

*This also shows one way to manage these long argument sequences.*

#### Fill Ellipse

```scheme
;; set solid fill brush color
(fill 10 100 100 255) 
 ;; x,y,w,h
(fill-ellipse 100 100 120 120)
(show)

```

A circle is an ellipse; were w and h are the same.

#### Fill Pie

```scheme
;; set solid fill brush color
(fill 100 0 100 255) 
;; x,y,w,h, start, end 
(fill-pie 100 100 120 120 0 100) 
(show)
```

### Fill with gradients

#### Gradient fill Rectangle

```scheme
(gradient
 10 10 50 50      ;; rectangle
 100 100 100 255  ;; colour 1
 200 200 100 255  ;; colour 2
 45.0 		      ;; angle
 #t )
;; x,y,w,h
(gradient-rect 210 210 80 80) 
(show)
```

#### Gradient Fill Ellipse

```scheme
(gradient
 10 10 100 100    ;; rectangle
 10  10 100 255   ;; colour 1
 100 0 100 255    ;; colour 2
 45.0 		      ;; angle
 #t )
(draw-ellipse 300 100 120 120) ;; x,y,w,h
(gradient-ellipse 300 100 120 120) ;; x,y,w,h
(show)
```

#### Gradient Fill Pie

```scheme
(gradient
 0 0 120 120    ;; rectangle
 10  0 100 255  ;; colour 1
 100 0 100 255  ;; colour 2
 70.0 		    ;; angle
 #t )
(gradient-pie 100 100 120 120 90 180) ;; x,y,w,h,start, end
(show)
```

------

### Text display

##### Fill String

```scheme
(fill 200 200 10 255)
(font-size 30)
(fill-string 20 20 "This is some text")
(show)
```

To draw text onto the graphics image; set the fill colour; set the font size and the draw the string. Text is painted using a brush; hence fill string.

##### Gradient String

```scheme
(paper 40 140 240 255) ;; paper color r,g,b, alpha
(clr 640 320) ;; clear 640x320 image	
(gradient
 0 0 20 20    ;; rectangle
 10  0 100 255  ;; colour 1
 100 0 100 255  ;; colour 2
 45.0 		   ;; angle
 #t )
(font-size 40)
(gradient-string 30 30 "This is some text")
(show)
```

------

### The Matrix

There is a drawing matrix that transforms what is being painted.

Parts of a drawing can be sheared; scaled and rotated; by applying changes to the matrix.

#### Transformations

Set up the image.

```scheme
(gradient 
 0 0 640 320 	;; rectangle
 50 50 50 255 	;; color 1
 100 100 100 255	;; color 2
 80.0		    ;; angle
 #t)
(clrg 640 320)	;; clear with gradient
```

Draw some text with the default matrix

```scheme
(fill 200 200 10 255)
(font-size 30)
(fill-string 20 20 "This is some text")
(show)
```

Try adding these transformations using the matrix commands

```scheme
(reset-matrix)  		   ;; clear the matrix
(rotate-at  0 0 10.0 )	   ;; rotate
(scale 1.2 2.0)			   ;; scale 
(translate -20.0 -20.0)    ;; translate
(shear 0.10 0.10) 		   ;; shear 
```

If you have some complex repeating shapes; these transformations could be used to move them around in an image.

------

### Show image on screen

*Should you need to look at what you are doing..*

#### Display mode

After any changes; use the **(show)** command to show the image in the on-screen view. 

The image is displayed using a mode below.

- The graphic mode 0..7. 
- This mode sets how the image is scaled to the image view.
- Modes 0 and 4 are most useful.

| Mode      | Effect      |         |
| --------- | ----------- | ------- |
| (gmode 0) | scale 1:1   | default |
| (gmode 1) | scale 1:2   |         |
| (gmode 2) | scale 1:4   |         |
| (gmode 3) | scale 2:1   |         |
| (gmode 4) | fill viewer | common  |
| (gmode 5) | scale 2:3   |         |
| (gmode 6) | scale 3:4   |         |
| (gmode 7) | scale 3:2   |         |

 You set the mode using **(gmode n)**

Mode 3 is handy when working on high DPI screens.



------

#### Display Quality

*You can adjust image quality with these functions*

| Mode                 | Effect | Note               |
| -------------------- | ------ | ------------------ |
| (fast-graphics)      | faster | jaggy              |
| (antialias-graphics) |        | not jaggy          |
| (smooth-graphics)    | Smooth | reasonable looking |

#### Draw a tree

```scheme
;; draw a tree in the image view.
;; based on example from rosetta code.

(define draw-tree
 (lambda ()
;;
    (define *scale* 10) 
    (define *split* 20)  
;;
    (define degrees->radians 
     (lambda (d)
        (let [ (pi 3.1415926535897932384626433832795028841)]
                (* d pi 1/180))))
;;
    (define (create-tree x1 y1 angle depth)
      (if (zero? depth)
        '()
        (let ((x2 (+ x1 (* (cos (degrees->radians angle)) depth *scale*)))
              (y2 (- y1 (* (sin (degrees->radians angle)) depth *scale*))))
          (append (list (map truncate (list x1 y1 x2 y2 depth)))
                  (create-tree x2 y2 (- angle *split*) (- depth 1))
                  (create-tree x2 y2 (+ angle *split*) (- depth 1))))))
;;
    (define tree (create-tree 320 500 90 9))
;;  
    (define draw-a-line 
        (lambda (x y x1 y1 s) 
          (pen-width (exact->inexact s))
          (draw-line 
            (inexact->exact (round x))
            (inexact->exact (round y))
            (inexact->exact (round x1))
            (inexact->exact (round y1)))))
;;
    (define get-line 
      (lambda (x) 
        (apply draw-a-line x)))
;;
    (colour 80 200 80 255)
    (paper 15 90 120 255)
    (clr 640 520) 
    (map get-line tree)
	(show)))
;;
(draw-tree)
```

------

## Random Lines

```Scheme
;;
(define random-lines
  (lambda (n)
    (dotimes n 
      (show) 
      (pen-width (+ 2.0 (random 6)))
      (colour (random 255) (random 255) (random 255) 255)
      (draw-line
        (random 800)
        (random 600)
        (random 800)
        (random 600)))))
;;
(paper 0 0 0 255)
(clr 800 600)

(random-lines 300)

 
```

## Animation 

#### The Repaint timer

There are three surfaces used by the image viewer; these are the display surface; the active surface and the screen surface. 

The active surface is used by all of the drawing commands; the display surface is only used to display the results.

When displaying the image view; a screen surface is used; this is filled with a checker board pattern first and then the display surface is drawn onto it; the combined image is then copied into the window.

The repaint timer when enabled overdraws only the changing 'display surface' part of the image viewer directly.

```Scheme
;; set the image viewer to redisplay itself at 30fps.
(set-repaint-timer 33)

(define random-lines
  (lambda (n)
    (dotimes n 
     ;; switch active and display; preserving.
	  (gswap 1)
      (pen-width (+ 2.0 (random 6)))
      (colour (random 255) (random 255) (random 255) 255)
      (draw-line
        (random 800)
        (random 600)
        (random 800)
        (random 600)))))
;;
(paper 10 20 100 255)
(clr 800 600)
(gswap 1)
;; shift-return on the line below 
(random-lines 300)
;;
```



[Animation](Animation.html)