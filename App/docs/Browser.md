 [Index](welcome.html)  

# Browser pane

This pane is used for documentation and help files.

This pane can contain code snippets like this

```Scheme

(define factorial
  (lambda (n)
 	(cond ((< n 0) #f)
          ((<= n 1) 1)
          (else (* n (factorial (- n 1)))))))

(factorial 10)

```



You can select and execute these using shift-return.

This pane can be useful for many things; such as displaying a help file; interacting with the programming scheme online book; or displaying some reports.  

All sorts of things are possible; such as creating and calling code in the browser and having browser scripts call back to Scheme; however I do not recommend this.

If you are looking to make extensive use of a browser; for example to create an App that uses a browser as its user interface; driven by Scheme behind the scenes;  there are more modern available options.



