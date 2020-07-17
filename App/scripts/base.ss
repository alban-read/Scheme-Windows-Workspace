;; base script
;; loads first; contains essentials only.

(define gc
  (lambda ()
    (collect)
    (collect)
    (collect)
    (collect)))
 (define >0 (lambda (x) (> x 0)))
 
(define full-path
 (lambda (path) 
    ((foreign-procedure 
      "GetFullPath" 
      (string) ptr) path)))
 
(define escape-pressed?
  (lambda () ((foreign-procedure "EscapeKeyPressed" () ptr))))

(define-syntax dotimes
  (syntax-rules ()
    [(_ n body ...)
     (let loop ([i n])
       (when (and (< 0 i) (not (escape-pressed?)))
         body
         ...
         (loop (- i 1))))]))

(define-syntax while
  (syntax-rules ()
    [(while test body ...)
     (let loop ()
       (if (and (not (escape-pressed?)) test)
           (begin body ... (loop))))]))

(define-syntax for
  (syntax-rules (for in to step)
    [(for i in elements body ...)
     (for-each (lambda (i) body ...) elements)]
    [(for i from start to end step is body ...)
     (let ([condition (lambda (i)
                        (cond
                          [(< is 0) (< i end)]
                          [(> is 0) (> i end)]
                          [else #f]))])
       (do ([i start (+ i is)]) ((condition i) (void)) body ...))]
    [(for i from start to end body ...)
     (do ([i start (+ i 1)]) ((> i end) (void)) body ...)]))

(define-syntax try
  (syntax-rules (catch)
    [(_ body (catch catcher))
     (call-with-current-continuation
       (lambda (exit)
         (with-exception-handler
           (lambda (condition) (catcher condition) (exit condition))
           (lambda () body))))]))

 
(define transcript0
  (lambda (x)
    ((foreign-procedure "append_transcript" (string) void) x)))

(define get-input-ed
  (lambda () ((foreign-procedure "getInputed" () ptr))))

(define set-input-ed
  (lambda (s)
    ((foreign-procedure "setInputed" (string) void) s)))

(define pretty-print-port pretty-print)

(define pretty-print-string
  (lambda (x)
    (with-output-to-string (lambda () (pretty-print-port x)))))

(define pretty-print-transcript
  (lambda (x)
    (transcript0
      (with-output-to-string (lambda () (pretty-print-port x))))))

(define display-port display)

(define newline-port newline)

(define display-transcript
  (lambda (x)
    (transcript0
      (with-output-to-string (lambda () (display-port x))))))

(define newline-transcript (lambda () (display-transcript #\newline)))

(define display
  (case-lambda
    [(x p)
     (unless (and (output-port? p) (textual-port? p))
       ($oops 'display "~s is not a textual output port" p))
     (display-port x p)]
    [(x) (display-transcript x)]))

(define display-string (lambda (x) (display x)))

(define newline
  (case-lambda
    [(p)
     (unless (and (output-port? p) (textual-port? p))
       ($oops 'display "~s is not a textual output port" p))
     (newline-port p)]
    [() (newline-transcript)]))

(define blank
  (lambda ()
    ((foreign-procedure "clear_transcript" () void))))

(define (println . args)
  (apply transcript0 args)
  (transcript0 "\r\n"))

 
(define evalrespond
  (lambda (x)
    ((foreign-procedure "eval_respond" (string) void) x)))
 

(define eval->string
  (lambda (x)
    (define os (open-output-string))
    (try (begin
           (let* ([is (open-input-string x)])
             (let ([expr (read is)])
               (while
                 (not (eq? #!eof expr))
                 (try (begin (write (eval expr) os) (gc))
                      (catch
                        (lambda (c)
                          (println
                            (call-with-string-output-port
                              (lambda (p) (display-condition c p)))))))
                 (newline os)
                 (set! expr (read is)))))
           (evalrespond (get-output-string os)))
         (catch
           (lambda (c)
             (println
               (call-with-string-output-port
                 (lambda (p) (display-condition c p)))))))))
 
(define eval->text
  (lambda (x)
    (define os (open-output-string))
    (try (begin
           (let* ([is (open-input-string x)])
             (let ([expr (read is)])
               (while
                 (not (eq? #!eof expr))
                 (try (begin (write (eval expr) os))
                      (catch (transcript0 (string-append "\r\n error: "))))
                 (newline os)
                 (set! expr (read is)))))
           (transcript0 (get-output-string os)))
         (catch (transcript0 (string-append "\r\n error: "))))))

 

;;; used to format text

(define eval->pretty
  (lambda (x)
    (define os (open-output-string))
    (try (begin
           (let* ([is (open-input-string x)])
             (let ([expr (read is)])
               (while
                 (not (eq? #!eof expr))
                 (try (begin (pretty-print-port expr os))
                      (catch
                        (lambda (c)
                          (println
                            (call-with-string-output-port
                              (lambda (p) (display-condition c p)))))))
                 (newline os)
                 (set! expr (read is)))))
           (display (get-output-string os)))
         (catch
           (lambda (c)
             (println
               (call-with-string-output-port
                 (lambda (p) (display-condition c p)))))))))
				 
(define format-scite
  (lambda () (eval->pretty (get-input-ed))))
				 
;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; simple graphics (some gdi plus commands)

(define show
  (lambda (n)
    ((foreign-procedure "graphicsUpdate" (int) ptr) n)))

(define clr
 (lambda (x y)
   ((foreign-procedure "CLRS"
    (int int ) ptr) x y)))

(define clrg
 (lambda (x y)
   ((foreign-procedure "CLRG"
    (int int ) ptr) x y)))

(define save-as-png
 (lambda (f)
   ((foreign-procedure "SAVEASPNG"
    (string) ptr) f)))

(define save-as-jpeg
 (lambda (f)
   ((foreign-procedure "SAVEASJPEG"
    (string) ptr) f)))

(define save-to-clipboard
 (lambda (f)
   ((foreign-procedure "SAVETOCLIPBOARD"
    (string) ptr) f)))

(define pen-width
 (lambda (w)
   ((foreign-procedure "PENWIDTH"
    (float) ptr) w)))

(define fill
 (lambda (a r g b)
   ((foreign-procedure "SOLIDBRUSH"
    (int int int int) ptr) a r g b)))

(define hatch
 (lambda (i a r g b a0 r0 g0 b0 )
   ((foreign-procedure "SETHATCHBRUSH"
    (int
	 int int int int
	 int int int int ) ptr) 
	 i a r g b a0 r0 g0 b0 )))

(define gradient
 (lambda (x y w h a r g b a0 r0 g0 b0 angle z )
   ((foreign-procedure "GRADIENTBRUSH"
    (int int int int
	 int int int int
	 int int int int 
	 double boolean) ptr) 
	 x y w h a r g b a0 r0 g0 b0 angle z)))

(define gradient-shape
 (lambda (shape focus scale)
   ((foreign-procedure "GRADIENTSHAPE"
    (string float float ) ptr) 
	 shape focus scale)))

(define paper
 (lambda (a r g b)
   ((foreign-procedure "PAPER"
    (int int int int) ptr) a r g b)))

(define draw-rect
 (lambda (x y w h)
   ((foreign-procedure "DRAWRECT"
    (int int int int) ptr) x y w h)))

(define fill-rect
 (lambda (x y w h)
   ((foreign-procedure "FILLSOLIDRECT"
    (int int int int) ptr) x y w h)))

(define rfill-rect
 (lambda (x y w h)
   ((foreign-procedure "REALFILLSOLIDRECT"
    (float float float float) ptr) x y w h)))

(define gradient-rect
 (lambda (x y w h)
   ((foreign-procedure "FILLGRADIENTRECT"
    (int int int int) ptr) x y w h)))

(define hatch-rect
 (lambda (x y w h)
   ((foreign-procedure "FILLHATCHRECT"
    (int int int int) ptr) x y w h)))

(define draw-ellipse
 (lambda (x y w h)
   ((foreign-procedure "DRAWELLIPSE"
    (int int int int) ptr) x y w h)))

(define fill-ellipse
 (lambda (x y w h)
   ((foreign-procedure "FILLSOLIDELLIPSE"
    (int int int int) ptr) x y w h)))

(define gradient-ellipse
 (lambda (x y w h)
   ((foreign-procedure "FILLGRADIENTELLIPSE"
    (int int int int) ptr) x y w h)))

(define hatch-ellipse
 (lambda (x y w h)
   ((foreign-procedure "FILLHATCHELLIPSE"
    (int int int int) ptr) x y w h)))

(define draw-arc
 (lambda (x y w h i j)
   ((foreign-procedure "DRAWARC"
    (int int int int int int) ptr) x y w h i j)))

(define draw-pie
 (lambda (x y w h i j)
   ((foreign-procedure "DRAWPIE"
    (int int int int int int) ptr) x y w h i j)))

(define fill-pie
 (lambda (x y w h i j)
   ((foreign-procedure "FILLSOLIDPIE"
    (int int int int int int) ptr) x y w h i j)))

(define gradient-pie
 (lambda (x y w h i j)
   ((foreign-procedure "FILLGRADIENTPIE"
    (int int int int int int) ptr) x y w h i j)))

(define hatch-pie
 (lambda (x y w h i j)
   ((foreign-procedure "FILLHATCHPIE"
    (int int int int int int) ptr) x y w h i j)))

(define draw-line
 (lambda (x y x0 y0)
   ((foreign-procedure "DRAWLINE"
    (int int int int) ptr) x y x0 y0)))

(define gradient-line
 (lambda (x y x0 y0)
   ((foreign-procedure "DRAWGRADIENTLINE"
    (int int int int) ptr) x y x0 y0)))

(define fill-string
 (lambda (x y s)
   ((foreign-procedure "DRAWSTRING"
    (int int string) ptr) x y s)))

(define gradient-string
 (lambda (x y s)
   ((foreign-procedure "DRAWGRADIENTSTRING"
    (int int string) ptr) x y s)))

(define set-pixel
 (lambda (x y)
   ((foreign-procedure "SETPIXEL"
    (int int) ptr) x y)))

(define rset-pixel
 (lambda (x y)
   ((foreign-procedure "RSETPIXEL"
    (float float) ptr) x y)))

(define colour
 (lambda (a r g b)
   ((foreign-procedure "COLR"
    (int int int int) ptr) a r g b)))

(define gmode
 (lambda (m)
   ((foreign-procedure "GRMODE"
    (int ) ptr) m)))

(define font-size
 (lambda (s)
   ((foreign-procedure "SETFONTSIZE"
    (int) ptr) s)))

(define fast-graphics
 (lambda ()
   ((foreign-procedure "QUALITYFAST"
    () ptr))))

(define smooth-graphics
 (lambda ()
   ((foreign-procedure "QUALITYHIGH"
    () ptr))))

(define antialias-graphics
 (lambda ()
   ((foreign-procedure "QUALITYANTIALIAS"
    () ptr))))

(define gflip
 (lambda (m)
   ((foreign-procedure "FLIP"
    (int ) ptr) m)))

(define reset-matrix
 (lambda ()
   ((foreign-procedure "MATRIXRESET"
    () ptr))))

(define invert-matrix
 (lambda ()
   ((foreign-procedure "MATRIXINVERT"
    () ptr))))

(define scale
 (lambda (x y)
   ((foreign-procedure "MATRIXSCALE"
    (float float) ptr) x y )))


(define translate
 (lambda (x y)
   ((foreign-procedure "MATRIXTRANSLATE"
    (float float) ptr) x y)))

(define shear
 (lambda (x y)
   ((foreign-procedure "MATRIXSHEAR"
    (float float) ptr) x y)))

(define rotate
 (lambda (a)
   ((foreign-procedure "MATRIXROTATE"
    (float) ptr) a )))

(define rotate-at
 (lambda (x y a)
   ((foreign-procedure "MATRIXROTATEAT"
    (int int float) ptr) x y a )))
	
;;; simple browser pane
;; please just use this for docs

(define navigate
 (lambda (url)
	((foreign-procedure "navigate" 
     (string) ptr) url )))

;; just dont; its not worth it.
(define connectSink
 (lambda (w b)
	((foreign-procedure "browserConnectSink" 
     (string string) ptr) w b)))

(define waitOnBrowser
 (lambda ()
	((foreign-procedure "waitOnBrowser" 
     () ptr))))

(define readInnerHTML
 (lambda ()
	((foreign-procedure "readInnerHTML" 
     () ptr))))

(define readInnerText
 (lambda ()
	((foreign-procedure "readInnerText" 
     () ptr))))

(define readTitle
 (lambda ()
	((foreign-procedure "readTitle" 
     () ptr))))

(define writeDocument
 (lambda (text)
	((foreign-procedure "writeDocument" 
     (string) ptr) text )))


(define loadDocument
 (lambda (fname)
	((foreign-procedure "loadDocument" 
     (string) ptr) fname )))

(define execBrowser
 (lambda (script)
	((foreign-procedure "execBrowser" 
     (string) ptr) script )))

(define callfunc
 (lambda (lst)
	((foreign-procedure "callfunc" 
     (ptr) ptr) lst )))


(define getElementsByID
 (lambda (lst)
	((foreign-procedure "getElementsById" 
     (ptr) ptr) lst )))
	 
(define clearDocument
 (lambda ()
	((foreign-procedure "clearDocument" 
     () ptr) )))

(define btofslash 
 (lambda (s) 
	((foreign-procedure "backtoforwardslash" 
	 (string) ptr) s )))
	 
(define help 
 (lambda ()
  (navigate (full-path "docs/readme.html"))))

(define welcome 
 (lambda ()
  (navigate (full-path "docs/readme.html"))))

(define license 
 (lambda ()
  (navigate (full-path "docs/license.html"))))


(define blank 
 (lambda ()
  (navigate "about:blank")))

;; this needs to execute fast
(define OnNavigate 
	(lambda (s)
	  #f))