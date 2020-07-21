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