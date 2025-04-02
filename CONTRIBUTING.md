# Contributing to konstruct

First off, thank you for considering contributing to konstruct! This document provides guidelines and steps for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [How Can I Contribute?](#how-can-i-contribute)
- [Styleguides](#styleguides)
- [Commit Messages](#commit-messages)
- [Pull Requests](#pull-requests)

## Code of Conduct

This project adheres to a Code of Conduct that all participants are expected to follow. Please read and understand it before contributing.

In short: be respectful, be inclusive, be collaborative.

## Getting Started

1. Fork the repository
2. Clone your fork locally
3. Set up the development environment
4. Create a branch for your changes
5. Make your changes
6. Test thoroughly
7. Submit a pull request

## How Can I Contribute?

### Reporting Bugs

- Check if the bug has already been reported
- Use the bug report template
- Include detailed steps to reproduce
- Include logs and error messages
- Specify environment details

### Suggesting Enhancements

- Check if the enhancement has already been suggested
- Use the feature request template
- Clearly describe the problem and solution
- Explain why this enhancement would be useful

### Pull Requests

- Fill in the required template
- Do not include issue numbers in the PR title
- Follow all instructions in the template
- Update documentation as needed

## Styleguides

### C Code Styleguide

- Use 4 spaces for indentation, not tabs
- Keep lines under 80 characters where possible
- Use descriptive variable and function names
- Comment your code:
  - Function headers should describe purpose, parameters, and return values
  - Complex logic should have inline comments
- Follow kernel naming conventions:
  - snake_case for functions and variables
  - ALL_CAPS for constants and macros

Example:

```c
/**
 * do_something - Brief description of function
 * @param1: Description of first parameter
 * @param2: Description of second parameter
 *
 * Detailed description of what the function does.
 *
 * Return: Description of return value
 */
int do_something(int param1, char *param2)
{
    int result = 0;
    
    // Complex logic explanation
    if (param1 > 10) {
        result = process_data(param2);
    }
    
    return result;
}
```

### Assembly Code Styleguide

- Use 4 spaces for indentation
- Include comments for each logical block
- Label names should be descriptive

### Documentation Styleguide

- Use Markdown for documentation
- Link related documentation where appropriate
- Include examples where helpful
- Keep documentation up to date with code changes

## Commit Messages

- Use the present tense ("Add feature" not "Added feature")
- Use the imperative mood ("Move cursor to..." not "Moves cursor to...")
- Limit the first line to 72 characters or less
- Reference issues and pull requests after the first line
- Consider starting the commit message with an applicable emoji:
  - 🐛 `:bug:` for bug fixes
  - ✨ `:sparkles:` for new features
  - 📚 `:books:` for documentation
  - 🧹 `:broom:` for code cleanup

## Pull Requests

1. Follow all instructions in the template
2. Include screenshots and animated GIFs when possible
3. Document new code thoroughly
4. End all files with a newline
5. Avoid platform-dependent code

Thank you for contributing to konstruct!
