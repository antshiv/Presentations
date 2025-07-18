		// Also available as an ES module, see:
		// https://revealjs.com/initialization/
		Reveal.initialize({
			controls: true,
			progress: true,
			center: true,
			hash: true,

			// Learn about plugins: https://revealjs.com/plugins/
			plugins: [RevealZoom, RevealNotes, RevealSearch, RevealMarkdown, RevealHighlight, RevealMath.KaTeX]
		});


		// use a script tag or an external JS file
		document.addEventListener("DOMContentLoaded", (event) => {
			gsap.registerPlugin(DrawSVGPlugin, MotionPathHelper, MotionPathPlugin, MorphSVGPlugin, Physics2DPlugin, SplitText, TextPlugin, SlowMo, CustomEase)
			// gsap code here!
		});