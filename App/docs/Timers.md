 [Index](welcome.html)  

## Timers

Normally when you enter Scheme in the evaluator pane; and execute it; it runs right away.

Only one thing ever runs in the Scheme engine at a time; if one script is running; other scripts will have to wait for it to finish; this is also true of any Scheme functions scheduled by timers; they all wait to run.

There is a timer function called **after** that can be used to run something later.

Where this command :-

```Scheme

(display "hello") (newline)

```

Runs right now; you could also schedule commands to run in 20 seconds time like this:-

```Scheme
(after 20000 (lambda () 
               (display "hello")
               (newline)))

```

You could write a function that runs; and then reschedules itself to run again later in two seconds time.

```Scheme
(define repeating 
  (lambda () (after 2000 repeating)
    (display "hello")(newline)))

(repeating)

```

You can continue running other short  functions interactively; and the timer function will keep repeating seemingly in the background; running whenever Scheme is not evaluating anything else.

This would keep running until you redefine the function not to repeat; when it stops.

```Scheme
(define repeating (lambda () '()))
```

Given that only one Scheme thread is ever running; this may seem to be of limited use; however it can be useful when you are also doing animation with the timers below.

This is neither concurrency (two things seem to run at once) or parallelism (two processers run things at the same time) it is really only scheduling.



## Timers related to animation

There are two special purpose timers related to animation of the image view.

These are the **repaint-timer** and the **every-function**

The general idea with computer animation is that you display the latest frame; while the viewer is looking at that; you draw the next one; behind the scenes.

The repaint-timer tells the app to repaint the image viewer; since the app thread overlaps and is separate from the Scheme thread; this timer does not block Scheme.

If part of an animation is the app displaying the latest frame; and part is the Scheme function creating the next frame; those functions can overlap with each other.

There is only one repeating every-function; it is intended to run a drawing function every N milliseconds; allowing an animation to create the next frame; it rotates the active and display images; it is entirely animation related.

These two timers allow you to write animations;  simple games; or perhaps graphical applications that use the image view interactively.

You can work on and edit your animation in the evaluator pane; while it is still running in the image pane; making changes for example to a simple game; while it runs.

 