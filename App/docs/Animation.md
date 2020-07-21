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



