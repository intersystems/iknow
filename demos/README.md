# iKnow demos

## Demo notebook

The `demo.ipynb` Jupyter notebook walks through a few basic demos, including 
browsing the raw output and the highlighting demo described below (leveraging
[colorama](https://github.com/tartley/colorama)).

After installing [Jupyter](http://www.jupyter.org/), you can launch the notebook
using the following command, after navigating to the `/demos` folder:

```Shell
$ jupyter notebook demo.ipynb
```

## Highlighting

The `highlight.py` script leverages [colorama](https://github.com/tartley/colorama) 
to add ASCII markup to the input text based on iKnow parsing results, emphasizing 
concepts and colouring based on the negation and certainty attributes.

```Shell
$ python highlight.py "This is not necessarily a great way of testing, but it works well enough!"
```

## Negation stripping

The `strip_negation.py` script takes a file path (optionally using wildcards) and
will print the contents of those files, **excluding** anything iKnow considers to
be in a negative content:

```Shell
$ python strip_negation.py *.txt
```

This output can then be piped to other tools such as linux' `grep` to search for
terms used in an affirmative context:

```Shell
$ python strip_negation.py "*.txt" | grep fix
```