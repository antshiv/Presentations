# Presentations

## Description
This repository contains presentations that are shown at the [Antshiv Robotics](https://www.youtube.com/channel/UCTHcIBTO5_Ly3GXAk_UccTQ) youtube channel. 

## How the presentations are created
The presentations are created using [Reveal.js](https://revealjs.com/). The presentations are written in HTML and CSS.
Animations present are made using html5 canvas and javascript. 

## How to run the presentations locally
Git pull the repository and navigate to the folder of the presentation you want to run. Then open index.html in your browser.
You do not need to install a server to run the presentations locally.

## Publish on GitHub Pages
This repository uses a small script to generate a root `index.html` that lists all presentation decks.

1. Rebuild the landing page:

`./scripts/build_index.sh`

2. Push this repository to GitHub.
3. In GitHub, open `Settings` -> `Pages`.
4. Set `Source` to `Deploy from a branch`.
5. Select your default branch (usually `main`) and folder `/ (root)`.
6. Save and wait for deployment.

Your public URL will be:

`https://<your-github-username>.github.io/<your-repo-name>/`

From that page, users can click any presentation and open it directly.




