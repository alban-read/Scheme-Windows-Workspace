;; fractal fern - scheme example
;; drag and drop into evaluator view 
;; draws into image pane.

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
