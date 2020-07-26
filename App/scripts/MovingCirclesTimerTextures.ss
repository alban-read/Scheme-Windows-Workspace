;; Simple animation in image view
;; using the two timers; and texture blitting

(clr 800 600)

(define circlecount 800)

(define make-circle-texture
 (lambda (c)
    (activate-bitmap (make-new-bitmap 64 64))
    (apply fill (append c '(128)))
    (apply colour (append c '(255)))
    (pen-width 1.5)
    (fill-ellipse 0 0 50 50)
    (draw-ellipse 0 0 50 50)
    (make-texture (clone-image (get-active)))))


(clear-all-textures)
(dotimes circlecount
 (make-circle-texture (list (random 255)(random 255)(random 255))))
 
 ;; instead of a random colour the circle now has a random texture id.
 (define newcircle
  (lambda ()
    (list
      (list (random 800) (random 600))
      (list (- 5 (random 10)) (- 5 (random 10)))
      (list (random circlecount)))))
 

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

;; draw a circle; by displaying its texture
(define drawcirc
 (lambda (c) 
    (apply rect-blit (append (caddr c) (car c) (list 52 52)))))
 

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
 
;; timer refresh
(set-repaint-timer 60)

;; run circle step on the repeating timer.
(set-every-function 1000 60 0 
		(lambda ()
		  (circle-step)(gc)))

 
