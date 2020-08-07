# Contributing

## Introduction
First off, thank you for considering contributing to EKAT.

Following these guidelines helps to communicate that you respect the time of the
developers managing and developing this open source project. In return, they
should reciprocate that respect in addressing your issue, assessing changes, and
helping you finalize your pull requests.

For more information on contributing to open source projects,
[GitHub's own guide](https://guides.github.com/activities/contributing-to-open-source/)
is a great starting point. Also, checkout the [Zen of Scientific Software Maintenance](https://jrleeman.github.io/ScientificSoftwareMaintenance/)
for some guiding principles on how to create high quality scientific software contributions.

## Getting Started

EKAT stands for E3SM Kokkos Application Toolkit. As the name suggests, it is a set of tools
that enable applications in the E3SM ecosystem to write Kokkos-enabled software.
In other words, we try to add common utilities here, so that we a) avoid rewriting the same
code multiple times, and b) provide E3SM applications with a well tested set of utilities
that are useful and easy to inject in their code. Utilities include actual code (mainly C++,
with some F90), but also [CMake](https://cmake.org/) scripts. 

## What Can You Do?
* Open an [issue](https://github.com/E3SM-Project/EKAT/issues) if something is not working,
  not clear, or if you think something could be improved.

* Open a [pull request](https://github.com/E3SM-Project/EKAT/pulls) if you already have
  some code you would like to share. It doesn’t need to be perfect! We will help you clean
  things up, test it, etc

## Code of conduct

If you decide to participate to EKAT, be aware that there are a few conduct rules. This section
should go without saying, but just for the sake of being clear, here they are:

* Do not use offensive language. Beside curse words, this includes any offensive remark
  (including racial, sexual, derogatory) directly or indirectly aimed at other people,
  regardless of whether they are internal to EKAT, or external people.
* Avoid any reference to politics or social issue. This is not the place for those discussions.
* Be gentle, patient and inclusive. Just because something is clear to you, it doesn't mean that
  it should be clear to others. Your point of view might not be the only one, or might even
  be inappropriate (or utterly wrong), so keep an open mind.
* It is ok to criticize someone's work/code in a consturctive way. That's what helps improving,
  and that's what discussions and code reviews are for. As long as the tone is acceptable,
  and the critiques are brought up in a constructive way, it all helps. Shaming and/or
  not constructive critiques are not helping at all.

All comments that do not respect these guidelines will be subject to editing, and possibly deletion.
If a user continues to violate conduct rules, she/he/they might be blocked and banned from the repo.

## Issues and Pull Requests

When creating a new issue, please be as specific as possible. We provide templates for bug, feature,
and question issues. Please, help us triaging your issue by using the proper category. Please,
follow the instructions on the issue template.

We also provide a template for Pull Requests, that you can use to, again, help us better understand
what the PR is doing. Once the PR is up, someone will look at it, and should comment within
a couple of days (could be more if everyone is on vacation). Notice that all PR *must* undergo a
review process in order to be merged. As for issues, please follow all the instructions in the template.
