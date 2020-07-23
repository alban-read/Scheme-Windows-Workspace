## Animation

 [Index](Readme.html)  

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

When using the repaint timer a script just needs to do its drawing and then swap over the active and display surfaces (using gswap.) 

(gswap 1) copies the current active surface to the new one; so that a picture builds incrementally.

(gswap 0) switches surfaces without copying the latest data; used for animation; where every scene is completely redrawn.

### Moving circles 

```Scheme
(set-repaint-timer 30)
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

In the code above we see that each step of the animation (for each frame created) is run by circle-step.

This is normal for an animation or a game; to have a loop; that repeats for each frame.

The problem is that running the step function literally in a loop; makes the timing of each frame variable depending on the work to do; and also prevents any other Scheme functions from running.

For this reason there is also a timer that calls a step function automatically in the background and swaps the surfaces.

To use it change the code to look like this:-

```Scheme
(define every_step 
	(lambda ()
		(circle-step)
      	(gc)
      ))

(set-every-timer 1000 60 0)

```

Also remove any (gswap n) functions as they now happen at the end of each step.

The function must be named  'every_step'; and in this case it calls the tested circle-step function.

The set-every-timer function here; is set to wait one second and then call the step function every 60 ms.

The step function needs to be barebones; well-tested; simple and fast.

It is used to only run the animation; check for keys; update animation state; it does not have the nice safe environment that you normally get when you are evaluating scheme.

if there is an error; it will crash.

The step function needs to complete quickly (it needs to be fast anyway to complete many frames a second.)

Note I added a call to (gc) garbage collect; otherwise the garbage will rapidly build up from calling the function thousands of times; and you will run out of memory.

Setting the times to 0 causes the timer to be stopped :-

```
(set-every-timer 0 0 0)
```

The nice thing is that assuming the function does finish quickly; there is plenty of time left over to run other commands; only one command runs at a time; but commands from the evaluator can sneak into the spare time available between steps.

So for example while running the animation in the viewer; you will run out of circles; however you can start another set of circles moving by re-executing:-

```Scheme
(define circles 
  (newcircles 700))
```

Although all the code is fast; it is native code compiled by C or by Scheme; Windows GDI plus is designed for quality not speed.

No software based rendering is very fast at drawing into a sixteen million colour display; let alone rendering that is meant to be accurate smooth and does alpha blending.

Faster computers are obviously better at drawing things in software. 

In this case the speed of your computer dictates how many circles can be redrawn in time.

```Scheme
;; Simple animation in image view
;; using the two timers

(clr 800 600)

(define circlecount 1000)

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
  (newcircles circlecount))

;; unjam any circle that is stuck	
(define unstickv 
 (lambda (v) 
	(list (if (= (car v) 0) (- 5 (random 10)) (car v))
		  (if (= (cadr v) 0) (- 5 (random 10)) (cadr v))))) 

(define count-offscreen
 (lambda ()
	(let ([count 0])
	 (for e in circles 
	  (when 
		(or 
		 (> (caar e) 800) 
		 (< (caar e) 0)
		 (> (cadar e) 600) 
		 (< (cadar e) 0))
			(set! count (+ count 1))))  count ))) 
			
(define all-off 
	(lambda ()
	 (>= (count-offscreen) circlecount)))



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
	(when (all-off) 
		(set! circles 
			(newcircles circlecount)))
	(set! circles (map move-circles circles))))

(define every_step 
	(lambda ()
		(circle-step)(gc)))

;; timer refresh
(set-repaint-timer 60)

;; run every animation step on the timer
(set-every-timer 1000 60 0)

 
```

Consider that to draw at 60FPS (the same as the standard slow monitor refresh) each step in the game has to finish in only 16ms; including the time to redraw the frame; this is why we have hardware accelerated graphics.  None the less; even in software; you can write a lot of fun 2D games on a fast computer.

