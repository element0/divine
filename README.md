# divine

The 'divine' tool - it generates HTML from an outline.

	red
	   green
	      blue

will generate:

	<div id="red">
	    <div id="green">
	        <div id="blue">
		</div>
	    </div>
        </div>

it has some "Classy" features such as:

	red .brick
	    green .plant
	        blue .bayou

will generate:


	<div id="red" class="brick">
	    <div id="green" class="plant">
	        <div id="blue" class="bayou">
		</div>
	    </div>
        </div>

it has some really NOT classy limitations:

	red .brick .warehouse

will _not_ add both classes -- please *FIXME*

But don't change the channel!  There's are _MIXINS_!

	red
	    $green

	green
	    blue

will also generate:
	
        <div id="red">
	    <div id="green">
	        <div id="blue">
		</div>
	    </div>
        </div>

And a very special caveat which can both help and harm: Please be advised that line breaks break the chain of love and leave the content unrendered.  This could be frustrating.  But it's actually very awesome.

	red
	    green
	    
	        blue

'blue' here doesn't render.  This is the *comment* style.

Regular text must be marked like this:

	red
	    green
	        "this is text, to be passed straight through

Notice there is no close quotation mark.  It's lazy.

Also, HTML or XML can be written using *only open tags*:

	<a href="#red">
	   "this text will be the link

That's all folks!
