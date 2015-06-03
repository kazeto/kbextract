# kbextract
Extracts knowledge from shared argument event pairs.

# example

    $ make
    $ zcat counts.pair.tsv.gz | grep "work-v:nsubj" | grep "make-v:nsubj" | ./extractor
    
