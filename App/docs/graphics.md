## Graphics

 [Index](Readme.html)  

#### <a name="image-pane">The image pane</a>

This displays an image; created typically by the GDI+ based drawing functions.

This pane exists as way to interactively test scripts that draw 2D images.

There are a couple of examples in the scripts folder; if you copy one and drop it onto the evaluator view it will draw a graphic. Also you can run through the examples here.

#### <a name="graphics-functions">Graphics functions</a>

These functions work on a single bitmap in memory; it does not need to be visible.

The image pane; when visible;  also display the bitmap as it is being drawn.

Windows includes a 2D software; alpha blending graphics library;  that provides the drawing functions.

 

- Set the app to Docking Image Layout and an image view will be displayed; your graphics commands can be displayed there.
- You can drag and drop an example from a scripts folder onto the Evaluator view; such as drawtree.ss which will draw a tree shape when evaluated.
- Or select a function from the browser pane and shift-run it.
- On  high DPI screens; it helps performance to keep the image pane; a reasonable size.

[contents](#contents)

------



## Graphics

##### A fractal fern

The Shell has an image viewer mode.

You can interactively create images and display them in the viewer.

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
    (show 0)))
;;
(define fern
  (lambda () (dotimes 100 (transform) (draw-point)) (show 0)))
;;
(init)
;; select and shift-return
(fern)
;;
```

The example displays a famous fractal fern.

Select Docking Image Layout from the menu.

Paste in the text; press .

```
<ctrl><enter>
```

Then select the (fern) command and press.

```
<shift><enter>
```

 to progressively drawn more of the fern pattern.

Using set-pixel is very slow; and not generally recommended. 

There are commands to draw lines and rectangles that are faster.

## Graphics 2D commands

- For 2D graphics Windows GDI+ provides smooth anti-aliased lines.
- The shell app has a view that displays an image.
- All graphics operations take place to a single **off-screen bitmap**.
- The image is displayed when the **show** function is called.
- The graphics functions are simple.

------

There is one image bitmap at any time that these commands write on.

------

*Set the docking mode to image layout in the shell.*

#### Create  an image

An image always has a background paper colour or gradient.

Set the paper colour to an r,g,b,a (red,green,blue,alpha) value; then create a blank image

```scheme
(paper 40 140 240 255) ;; paper color r,g,b, alpha
(clr 640 320) ;; clear 640x320 image		
(show 0) ;; show on screen
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
(show 0)
```

To re-display a changed image on the image view

The parameter for show (0-3) sends the update message to different parts of the application window; forcing more or less of the screen to refresh.

```scheme
(show 0) ;; show the image
```

------

#### Save the image to a file

```scheme
(save-as-png "myimage.png")
(save-as-jpeg "myimage.jpeg")
```

------

### Colours

- Colours are set for pens and for brushes.
- The colours are set using red,green,blue, alpha values (0..255).
- Alpha is used to blend.
- There is one pen; set using **colour**.
- A solid brush also has a colour; there is a paper brush; a fill brush and a gradient brush used by different commands.
- A linear gradient brush has two colours and an angle.
- You set the relevant brushes using the **paper**, **fill** and **gradient** procedures.



[contents](#contents)

------

### Drawing on an image

The following commands draw onto the image.

*Remember that no changes are displayed on the screen until you re-show the image*

### Lines

#### Line drawing

A line has a colour and a pen width

```scheme
(colour 0 200 0 255) ;; green 
(pen-width 2.2)	   ;; wide line	
(draw-line 10 10 200 200) ;; draw a line
(show 0)
```

#### Rectangle drawing

```scheme
(colour 200 0 200 255)  ;; purple
(pen-width 3.5)	   ;; wide line	
(draw-rect 10 10 40 40) ;; x,y, width height
(show 0)
```

#### Ellipse Drawing

```scheme
(colour 0 0 200 255) ;; blue 
(pen-width 0.5) ;; narrow line
(draw-ellipse 10 10 50 50) ;; x,y, width, height
(show 0)
```

#### Pie Drawing

```scheme
(paper 40 140 240 255) ;; paper color r,g,b, alpha
(clr 640 320) ;; clear 640x320 image		
(colour 0 200 200 255) ;; set colour
(pen-width 8.0)
(draw-pie 120 100 150 150 0 100) ;; x,y,width,height,start,end
(show 0)
```

#### Arc Drawing

```scheme
(colour 200 100 200 255) ;; set colour
(pen-width 4.0)
(draw-arc 10 10 150 150 0 100) ;; x,y,width,height,start,end
(show 0)
```

### Fill shapes

A shape has a solid fill brush or a gradient fill brush

#### Fill Rectangle

```scheme
(fill 100 0 100 255) ;; set solid fill brush color
(fill-rect 100 100 50 50) ;; x,y,w,h
(show 0)
```

#### Fill Ellipse

```scheme
(fill 10 100 100 255) ;; set solid fill brush color
(fill-ellipse 100 100 120 120) ;; x,y,w,h
(show 0)

```

A circle is an ellipse; were w and h are the same.

#### Fill Pie

```scheme
(fill 100 0 100 255) ;; set solid fill brush color
(fill-pie 100 100 120 120) ;; x,y,w,h
(show 0)
```

### Fill with gradients

#### Gradient fill Rectangle

```scheme
(gradient
 10 10 50 50    ;; rectangle
 100 100 100 255  ;; colour 1
 200 200 100 255  ;; colour 2
 45.0 		   ;; angle
 #t )
(gradient-rect 210 210 80 80) ;; x,y,w,h
(show 0)
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
(show 0)
```

#### Gradient Fill Pie

```scheme
(gradient
 0 0 120 120    ;; rectangle
 10  0 100 255  ;; colour 1
 100 0 100 255  ;; colour 2
 70.0 		   ;; angle
 #t )
(fill-pie 100 100 120 120 90 180) ;; x,y,w,h,start, end
(show 0)
```

------

### Text display

##### Fill String

```scheme
(fill 200 200 10 255)
(font-size 30)
(fill-string 20 20 "This is some text")
(show 0)
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
(show 0)
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
(show 0)
```

Try adding these transformations using the matrix commands

```scheme
(reset-matrix)  		   ;; clear the matrix
(rotate-at  0 0 10.0 )	   ;; rotate
(scale 1.2 2.0)			  ;; scale 
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
;; draw a tree in image view.
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
        (if (= 1 (random 4)) (show 0 )) ;; watch lines
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
    (show 1)
    (map get-line tree)
    (show 2)))
;;
(draw-tree)
```

------

