# Workspace - tiles

 [Index](Readme.html)  

------

### <a name="the-gui">The workspace</a>

Provides a tiled workspace; based on three text panes; a graphics pane; and an html pane.

The tiled panes can be rearranged into a few common arrangements using the Docking menu.

Assuming a default layout; you enter an expression in the top evaluator pane.

When you execute the expression the results are displayed in the lower pane; any output is printed on the transcript to the right.

The script, its results and its output are separated. 

These are all scrolling text views.

You can select any expression you want to run; and press **shift+return** to run it.

Or you can run the entire view; using **control+return**.

The text panes support drag and drop; you can drag a scheme script file for example into a pane.  You can also drag in selected text.

Scheme scripts are syntax highlighted and brackets are matched.



**Not a file editor**

- The tiles are not backed to a file anywhere; they are for interacting with Scheme.
- They are also cleared when you re-arrange the layout using the docking menu.
- When working I typically test functions in the evaluator pane; while adding them to a file open in an editor. I use notepad++.



### <a name="browser-pane">The browser pane</a>

This pane exists for application documentation (such as this document.)

Selecting some text and pressing **shift-return** in the browser pane evaluates the selection.

This browser view is fully programmable; I do not recommend taking advantage of this; there are better ways to drive a browser from scheme.  

 [Index](Readme.html)  

------

#### <a name="image-pane">The image pane</a>

This displays an image; created typically by scripts using the GDI+ based drawing functions.

This pane exists as way to interactively test scripts that draw 2D images.

There are examples (fern, tree) in the scripts folder; if you drag and drop one onto the evaluator view; then evaluate it; they will draw a sample image.

#### <a name="graphics-functions">Graphics functions</a>

Use the GDI+ library; a software based; alpha blending graphics library.

- Use Image Layout to display the graphics pane; your graphics commands will be displayed there.

 [Index](Readme.html)  



---

#### <a name="text-panes">Text editor panes</a>

These are the main point of the app; these are modern minimal windows editors; with bracket matching; using the Scintilla text editing component with some custom logic. 

- Brackets are matched in scheme code as you move the cursor.

- Results are displayed in the results pane.

- values that are printed are displayed in the transcript.

- The Transcript view displays some timing information


##### Commands in the editor pane

Arrow keys move in the pane.

You clear a pane with <control><l> as you may expect.

In the example above if you <control><return> the whole pane will be evaluated.

In the example above if you select just the fib execution; you can <shift><return>  to execute just that function.

The main thing I miss is in the editor is a code formatting function; so I plan to add that one day.

This editor has the sort of retro commands you might expect from a 1990s windows application; rather than a 1970s vintage teletype. Lets not judge one other.

##### Zoomable

- The text view zooms in and out; use control and mouse wheel.
- The html view does the same.

 [Index](Readme.html)  

 

------

#### Keys in the text panes

The text panes I believe like a normal windows app.

**Arrow keys** move the cursor; most keys insert a character at the cursor.

**ctrl-d** duplicates the current line.

**pageup pagedown** move up and down the pane.

**home end** move to start and end of lines.

**shift-end** selects a line

**ctrl-home ctrl-end** move to start and end of text.

**ctrl-shift-home ctrl-shift-end** move to start or end of text; and select it.

**ctrl-arrow** will jump along the line.

**ctrl-shift-arrow** will jump along the line; selecting.

**tab** moves right 4 spaces.

**insert** switches between insert and overtype modes; cursor changes.

**delete**  deletes forward

**backspace** deletes back

**shift-return** evaluates the selection

**ctrl-return** evaluates the pane.

---



## <a name="the-escape-key">Escape key handler </a>

*Be mindful; that some code can run away; and never stop; which will require you to kill the app.*

If you run a script you may want to also interrupt it early.

- The Terminal responds to <ctrl>+<c> signals; so you can interrupt any running function.
- I would very much like to send that same interrupt to the engine when escape is pressed which would solve some problems below; if you know how to do that; please let me know.
- Meanwhile library and user code *should check for the escape key*.

You need to add a check for the escape key into any loops you write in your scripts.

For example this is an infinite loop; that will start to count up to infinity; not a good thing to tell a computer to do.

```scheme
; !!! do not run this. it never ends.
!(let loop ((i 1))
  (display (number->string i)) 
  (newline)
  (loop (+ 1 i)))
```

If you run this is in a terminal you could hit <control><c> and it would stop.

However this will run *in the app for ever*; since killing a running thread is a very bad thing to do; as it will often just lead to a subsequent crash. 

This example is especially bad; as it uses the scheme engines thread and also overwhelms the application by sending infinite messages to the transcript pane.

Usually the workspace is not impacted by a run away script but in this case it would be.

To avoid never ending scripts from running away I often need to add an escape key check to any functions that might repeat forever.

```scheme
(when (escape-pressed?) (raise "Escape Key!"))
```

So we have :-

```scheme
(let loop ((i 1))
  (when (escape-pressed?) (raise "Escape Key!"))
  (display (number->string i)) 
  (newline)
  (loop (+ 1 i)))
```

The predefined **dotimes** and **while** commands already check for escape; and a for loop is not infinite. A pattern with scheme is to use explicit recursion for all loops; that pattern make this hazard more likely; remember to add this to your code if you want it to be interruptible.

```scheme
(when (escape-pressed?) (raise "Escape Key!"))
```

 

 [Index](Readme.html)  