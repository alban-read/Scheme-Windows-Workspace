## Animation

  [Index](welcome.html)    [Timers](Timers.html) [Textures](Images and Textures.html)

The general idea with computer animation is you show the viewer the latest frame and while they are looking at that, you draw the next one; behind the scenes.

#### The Repaint timer

There are three surfaces used by the image viewer; these are the display surface; the active surface and the screen surface. 

The active surface is used by all of the drawing commands; the display surface is only used to display the results.

When displaying the image view; a screen surface is also used; this is filled with a checker board pattern first and then the display surface is drawn onto it; finally the combined image is then copied into the window.

The repaint timer is a repeating timer that periodically overdraws only the changing 'display surface' part of the image viewer directly. This is more efficient than the normal paint routine.

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

After you start the repaint timer; your script just needs to do its drawing and swap surfaces.  

| (gswap 0) | used for animation; where every scene is completely redrawn. |
| --------- | ------------------------------------------------------------ |
| (gswap 1) | copies the current active surface so that a picture builds incrementally. |

### Moving circles 

```Scheme
(set-repaint-timer 33)
(clr 800 600)

;; make a new circle
(define newcircle
  (lambda ()
    (list
      (list (random 800) (random 600))
      (list (- 5 (random 10)) (- 5 (random 10)))
      (list (random 255) (random 255) (random 255)))))

;; make n new circles
(define newcircles 
  (lambda (n) 
	(let ([l '()])
	  (dotimes n 
		(set! l (append l (list (newcircle))))) l)))

;; keep a list of circles
(define circles 
  (newcircles 700))

;; unjam any circle that is stuck	
(define unstickv 
 (lambda (v) 
	(list (if (= (car v) 0) (- 5 (random 10)) (car v))
		  (if (= (cadr v) 0) (- 5 (random 10)) (cadr v))))) 

;; move all circles
(define move-circles
 (lambda (c)
 (list (map + (car c)(cadr c)) (unstickv (cadr c)) (caddr c))))

;; draw a circle
(define drawcirc
 (lambda (c) 
	(apply fill (append (caddr c) (list 128)))
	(apply colour (append (caddr c) (list 255)))
	(pen-width 1.5)
    (apply fill-ellipse (append (car c) (list 50 50)))
	(apply draw-ellipse (append (car c) (list 50 50)))))

;; perform one step
(define circle-step
 (lambda ()
	(fill 0 0 0 255)
	(fill-rect 0 0 800 600)
	(map drawcirc circles)
	(gswap 0)
	(set! circles (map move-circles circles))))

;; perform 1000 steps	 
(dotimes 1000 
 (circle-step))


```

Shows the effect of using (gswap 0).

## Automating the step function

In the code above we see that each step of the animation (each frame created) is performed by circle-step.

You can run the sequence one step at a time; single stepping it; using shift-return.

```Scheme
;; add more circles
(define circles 
  (newcircles 700))

;; single step
(circle-step)

;; a lot of steps
(dotimes 1000 
 (circle-step))

```

This loop runs enough steps to display part of the animation.

It is normal for an animation or a game; to have a game loop.

The problem with running the steps in a loop is the timing of each frame will vary.

You can see this; when there are fewer circles the animation speeds up and they rush faster off the display.

The loop also prevents any other Scheme functions from running; killing off our interactivity.

For this reason there is a repeating timer that can be used schedule the step function automatically.

It also swaps the surfaces.

```Scheme
;; wait 1000ms run every 60ms; swap and clear.
(set-every-function 1000 60 0 
		(lambda ()
		  (circle-step)(gc)))
```

Note I added a call to **gc** garbage collect; otherwise garbage will rapidly build up from calling the function thousands of times.

The timer auto rotates the active and display images;  the Scheme function should longer swap them.

```Scheme
;; perform one step - no need to swap.
(define circle-step
 (lambda ()
	(fill 0 0 0 255)
	(fill-rect 0 0 800 600)
	(map drawcirc circles)
	(set! circles (map move-circles circles))))
```



When running the circle animation in the viewer; you will probably run out of circles; however you can just recreate them and the animation will continue.

```Scheme
(define circles 
  (newcircles 700))
```

As  a rule the step function needs to be barebones; well-tested; simple and fast.

It is used to only run the animation; it should check for keys; update animation state.

if there is an error; it may well crash.

The step function needs to complete quickly (it has to be fast to draw many frames in a second.)

### Stopping 

Setting the times to 0 causes the timer to be stopped :-

```
(set-every-function 0 0 0 '())
```

The nice thing is that assuming the function does finish quickly; there is plenty of time left over to run other commands in-between; as above we could add more circles when we ran out.

Only one scheme command runs at a time; commands from the evaluator have to sneak into the spare time available between steps



 

