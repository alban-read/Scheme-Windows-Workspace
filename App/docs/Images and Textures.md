 [Index](welcome.html)  

# Images and Textures for animation

In the animation section we moved some circles; circles are pretty simple; and we just drew each one.

Drawing each one is probably not the fastest way to move lots of circles around the screen.

This topic discusses how to display things more quickly.

- There is some support for images and textures in GDI+; where a texture is a brush that uses an image.
- And images can be drawn into the display in a couple of different ways.

As a starting point we can try just drawing the circles; as in the animation topic.

```Scheme
(paper 0 0 0 255)
(clr 800 600)

(define draw-a-circle 
 (lambda (x y) 
	(fill 200 0 0 128)
	(colour 200 0 0 255)
	(pen-width 1.5)
	(fill-ellipse x y 50 50)
	(draw-ellipse x y 50 50)))

(time 
 (dotimes 10000 
   (draw-a-circle (random 800)(random 600))))

(show)

```

The time command prints results on the transcript.

The results for me are around 0.31 - 0.32 which seems quick.

#### Creating a new image from a drawing

```Scheme
(paper 0 0 0 255)
(clr 800 600)
```

Create a blank surface first; as this also initializes various drawing routines for us.

```Scheme
(define red-circle
 (begin
	(activate-bitmap (make-new-bitmap 64 64))
	(fill 200 0 0 128)
	(colour 200 0 0 255)
	(pen-width 1.5)
	(fill-ellipse 0 0 50 50)
	(draw-ellipse 0 0 50 50)
	(clone-image (get-active) )))
```

This is a function that creates an image of a red circle.

The way we do this is to create a small bitmap; and activate it; so the drawing commands will now draw onto it.

Then we draw the circle onto it; setting the fill and pen colours; then filling the ellipse and drawing its edge.

lastly we copy the image back out of the active surface and assign it as red circle.

```Scheme
(show)
```

It is interesting to show the image now; the display will be a checkerboard; with our transparent circle drawn in the top left corner.  That is because our background is transparent.

```Scheme
(paper 0 0 0 255)
(clr 800 600)
(show)

```

Now we have an image of a red circle; we can draw it 10000 times and time that.

```
(time
 (dotimes 10000
  (draw-image red-circle (random 800) (random 600))))
(show)
```

This is one and a half times as fast at 0.23-024.

### Drawing using a texture

We can  turn the image into a texture; which can be used as a brush.

```Scheme
(texture red-circle)
```

This is especially fast when you have lots of identical images; you need to draw all the red circles; then all the green circles etc.

```Scheme

(paper 0 0 0 255)
(clr 800 600)
(show)

;; draw the red-circle textures and display them
(dotimes 1000
 (brush-rect (random 800) (random 600) 60 60))

(show)
```

That did seem; quick - but lets compare and measure it; using the time function.

```Scheme
(time
 (dotimes 10000
  (brush-rect (random 800) (random 600) 60 60)))
```

I get 0.11 - 0.12 for the pure drawing of 10000 circles by using them as a brush.

This is nearly three times faster than just drawing them; although significantly less flexible.

The more complicated a thing is we want to draw 10,000 of - the better the pay back should be.

#### Convenience functions

As we have seen textures are much faster to plot than images.

The difference is quite dramatic when you start moving a lot of sprites on a screen.

Some convenience functions are provided for creating and switching between images

You can load an image from a jpg or png file.

```Scheme
(define alien
 ( load-image "images//a1.png"))
(define alien-texture (make-texture alien))
```

Once an image is loaded you can convert it into a texture; the make-texture function; returns an id number.

```Scheme
(define tx -400)
(define ty -400)
(rect-blit tile-texture tx ty 1200 1200 ) 
```

The rect-blit function takes the id; and blits the texture with the id into the active surface.

This allows you to load many different images and display them; reasonably rapidly.

The textures are held in an array on the C side; you can make a thousand of them; which is more than I ever need; you can always recompile the code; to add some more.







