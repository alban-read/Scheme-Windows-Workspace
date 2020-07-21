;; Simple animation in image view
;; using the two timers

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
  (newcircles 1000))

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

(define every_step 
	(lambda ()
		(circle-step)(gc)))

;; timer refresh
(set-repaint-timer 30)

;; run every animation step on the timer
(set-every-timer 1000 60)

 
