
# Guidelines for collaboration on the iKnow GitHub repository

## Guidelines for branching

If you are not sure that you will be able to work on a specific task until the task is finished, i.e. without having to synch with the remote master before your work is finished, create a (local) branch. In that branch, follow the below guidelines for commits.

When pushing a branch with finished work into the remote master branch, consider whether you want to keep the intermediate commits for the future or not. If not, do a "squash merge".  

If you want to keep the intermediate steps, use "three-way merge", so the intermediate steps stay on a side-way in the history.  


## Guidelines for a good commit content

A good commit comprises a limited piece of work. 

If you work on different parts of the project for different reasons, e.g. fixing a bug in a Python script and adding lexreps to a language model, create separate commits that each reflect one coherent task. 

If you work on a large task with different components, try to commit your work intermittently in logical (local) commits. Preferably, each individual commit contains a fully working version so you can roll back in a meaningful way if needed, but of course the git police doesn’t check your local drive. Clean up an accidental messy local history before pushing your changes to a remote branch (e.g. using “squash commits”).

When a commit you push to a remote branch is part of a bigger whole, it may make sense to create an issue (e.g. labelled as "enhancement") if there is none already and link to it in the commit message. Issues make it easier to group commits and document them as a whole.


## Guidelines for a good commit message

Based on https://chris.beams.io/posts/git-commit/  
See also the chapter "Good practice" on  https://github.com/RomuloOliveira/commit-messages-guide for examples.

1.	Separate subject from body with a blank line.

2.	Limit the subject line to 50 characters.  
Japanese characters count double.  
Exception: automatically generated messages, starting with "[autoupdate-...]".

3.	Capitalize the subject line.

4.	Do not end the subject line with a period.

5.	Use the imperative mood in the subject line.  
A properly formed Git commit subject line should always be able to complete the following sentence:   
                   *If applied, this commit will* \<your subject line here\>

6.	Wrap the body at 72 characters.   
Japanese characters count double.

7.	Use the body to explain what and why vs. how.

8.	If the commit is related to an issue, make sure to refer to that issue.  
Git can link the issue and the commit if you write "fix #xxxx", "close #xxxx" or "resolve #xxx" in the message (plus some other phrases that are not mentioned here because they don’t comply with rule 5).
See https://docs.github.com/en/github/managing-your-work-on-github/linking-a-pull-request-to-an-issue for more information.

9.	Be generous with @mentions.   
Simple repository subscriptions don’t warn you on each and every commit, so if you want your coworkers to be notified, just make it explicit.


