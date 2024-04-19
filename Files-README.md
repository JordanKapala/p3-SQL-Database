---
layout: spec
sitemapOrder: 30
---

# Files README
{: .primer-spec-toc-ignore }

## New Data Structure

The new data structure that you'll need for Project 3 is the
unordered_map<>.  We'll be talking about how it works the first
lecture back after the midterm, but we didn't want you to have
to wait to start on your project until those lectures.  Look up the
online documentation on the unordered_map<>, and also look at the
example file on Canvas / Files / Sample Code / umap_demo.cpp
(taken from the Lecture slides) and wordcount.cpp.


## Starter Files

There are two starter files that you will need: TableEntry.h and
TableEntry.cpp.

Also look at the file Error_messages.txt.  There are only two "standard"
error messages for this project that will be sent to `cerr` (the other
errors, such as a table name that doesn't exist, are sent to `cout`).  You
will be shown your `cerr` output if:

1. It is not an INV test case (supposed to exit 1)
2. Your exit status is 1
3. The first line of text send to cerr matches a line in that file

If you want to add a second line, that's fine, but we won't display it to
you. For example, this is valid:

```cpp
cerr << "Error: Unknown command line option" << endl;
cerr << "  The argument seen was: " << choice << endl;
exit(1);
```

In this case, we would show you the first line of output in the autograder
feedback.


## Sample Files

Don't run long-quiet-input.txt without the -q/--quiet flag!
It produces a huge output file, and takes a long time to do it.

Execution times for our solution, running on the autograder:
    short-input.txt: 0.046s
    long-quiet-input.txt: 2.213s

We've also give you the specification input and output, along with three
"checkpoint" tests (input and output).  There is no required date for
completing the checkpoints, but these test cases do exist on the autograder
and will help you gauge your progress.
