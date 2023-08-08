const RotationDirection = {
	CLOCKWISE: 1,
	COUNTERCLOCKWISE: -1,
};

const LinearMovement = {
	UP: 1,
	DOWN: -1,
	UPDOWN: 0,
};

const HorizontalMovement = {
	LEFT: 1,
	RIGHT: -1,
};

const ForceDirection = {
	UP: 1,
	DOWN: -1,
};

console.log('script loaded');
quadcopter_path = "assets/quadcopter/2048w/quadcopter.png";
quadcopter_front_path = "assets/quadcopter/2048w/quadcopter_front.png";
propeller_path = "assets/propellers/2048w/propeller.png";
quadcopter_chassis_path = "assets/quadcopter/2048w/chassis.png";
propeller_circle_path = "assets/propellers/2048w/propeller_circle.png";


function populateCanvas(canvas_id, image, RotationDirectionLocal = null, LinearMovementLocal = null, HorizontalMovementLocal = null) {
	var canvas = document.getElementById(canvas_id);
	if (!canvas) {
		console.log('canvas not found');
		return;
	}
	var context = canvas.getContext("2d");
	if (!context) {
		console.log('context not found');
		return;
	}

	image.onload = function (event) {
		const centerX = canvas.width / 2;
		const centerY = canvas.height / 2;

		// Initial position of the image at the center of the canvas
		let x = (canvas.width - image.width) / 2;
		let y = (canvas.height - image.height) / 2;

		// Initial rotation angle (in degrees)
		let rotationAngle = -1;

		// Initial movement direction
		let direction = 1; // 1 for down, -1 for up

		var aspectRatio = image.width / image.height;
		context.drawImage(image, 0, 0, canvas.width, canvas.width / aspectRatio);

		// Function to draw the image at the given angle
		function drawRotatedImage() {
			context.clearRect(0, 0, canvas.width, canvas.height);

			// Save the current context state
			context.save();

			// Set the transform origin to the center of the image
			context.translate(centerX, centerY);
			context.rotate((rotationAngle * Math.PI) / 179); // Convert degrees to radians

			// Draw the scaled image with the top-left corner at the center
			const scaleFactor = 0.2; // Scale factor to half the size
			const scaledWidth = image.width * scaleFactor;
			const scaledHeight = image.height * scaleFactor;

			// Draw the image with the top-left corner at the center
			context.drawImage(image, -scaledWidth / 2, -scaledHeight / 2, scaledWidth, scaledHeight);

			// Restore the original context state
			context.restore();
		};

		// Function to draw the image at the current position
		function drawUpdatedPosition(scaledWidth, scaledHeight, yLocal) {
			context.clearRect(0, 0, canvas.width, canvas.height);

			// Save the current context state
			context.save();
			context.translate(centerX, centerY);

			// Set the transform origin to the center of the image
			// range is -canvan.height /2 to scaledHeight / 2
			context.drawImage(image, -scaledWidth / 2, yLocal, scaledWidth, scaledHeight);
			context.restore();
		}

		function updatePosition() {
			if (LinearMovementLocal === null) {
				return;
			}
			const scaleFactor = 0.2; // Scale factor to half the size
			const scaledWidth = image.width * scaleFactor;
			const scaledHeight = image.height * scaleFactor;

			// Change the image's position based on the direction
			y += 2 * direction; // Change the movement speed here (2 pixels per frame in this example)

			// Check if the image has reached the top or bottom of the canvas
			if (y <= -canvas.height / 2 || y >= scaledHeight / 2) {
				direction *= -1; // Reverse the direction when reaching the canvas boundaries
			}

			drawUpdatedPosition(scaledWidth, scaledHeight, y); // Redraw the image with the updated position
			requestAnimationFrame(updatePosition);

		}

		// Function to continuously rotate the image
		function animate() {
			if (RotationDirectionLocal === null) {
				return;
			}
			rotationAngle -= RotationDirectionLocal; // Change the rotation speed here (0 degree per frame in this example)
			drawRotatedImage();

			requestAnimationFrame(animate);
		};

		// Start the animation
		animate();
		updatePosition();
	};

	image.onerror = function (event) {
		console.log('image.onerror: Most likely path is not correct');
	};
};

function drawAnimateQuadcopter(canvas_id, quadcopterImageSrc, propellerImageSrc) {
	const canvas = document.getElementById(canvas_id);
	const context = canvas.getContext("2d");
	let chassisImage = new Image();
	chassisImage.src = quadcopterImageSrc;
	let propellerImage = new Image();
	propellerImage.src = propellerImageSrc;
	proppellerWidthHeight = 100;
	let rotationAngle = 0;

	const centerX = canvas.width / 2;
	const centerY = canvas.height / 2;

	// Set the transform origin to the center of the image
	context.rotate((-1 * Math.PI) / 179); // Convert degrees to radians

	const scaleFactor = 0.12; // Scale factor to half the size
	const scaledWidth = chassisImage.width * scaleFactor;
	const scaledHeight = chassisImage.height * scaleFactor;

	const propellerScaleFactor = 0.07; // Scale factor to half the size
	const scaledWidthPropeller = propellerImage.width * propellerScaleFactor;
	const scaledHeightPropeller = propellerImage.height * propellerScaleFactor;
	context.drawImage(chassisImage, -scaledWidth / 2, -scaledHeight / 2, scaledWidth, scaledHeight);


	// Function to draw the motor chassis with propellers
	function drawMotorChassis(image, x, y, height, width, rotationDirection) {
		context.save();
		//context.translate(x, y);
		context.translate(centerX + x, centerY + y);
		context.rotate((rotationDirection * Math.PI) / 180); // Convert degrees to radians

		// Draw the chassis image at position (100, 150)

		// Draw the propeller images at their respective positions
		if (propellerImage.complete) {
			//context.drawImage(propellerImage, - scaledWidthPropeller /2 - (scaledWidth /2), -scaledHeightPropeller /2 - (scaledHeight /2 ), scaledWidthPropeller, scaledHeightPropeller);
			//context.drawImage(propellerImage, - scaledWidthPropeller /2 - (scaledWidth /2), -scaledHeightPropeller /2 + (scaledHeight /2 ), scaledWidthPropeller, scaledHeightPropeller);
			//context.drawImage(propellerImage, - scaledWidthPropeller /2 + (scaledWidth /2), -scaledHeightPropeller /2 - (scaledHeight /2 ), scaledWidthPropeller, scaledHeightPropeller);
			//context.drawImage(propellerImage, - scaledWidthPropeller /2 + (scaledWidth /2), -scaledHeightPropeller /2 + (scaledHeight /2 ), scaledWidthPropeller, scaledHeightPropeller);
			//context.drawImage(image, x,y, height, width);
			context.drawImage(image, -width / 2, -height / 2, width, height);
			//console.log("x: " + x + " y: " + y + " height: " + height + " width: " + width);
		}
		context.restore();
	}

	console.log("scaledWidthPropeller: " + scaledWidthPropeller);

	function animate() {
		// Increase the rotation angle each frame
		rotationAngle -= 1;
		// Draw everything on the canvas
		context.clearRect(-20, 0, canvas.width + 500, canvas.height + 500);
		context.save();
		context.translate(centerX, centerY);
		if (chassisImage.complete) {
			//context.drawImage(chassisImage, 0, 0, canvas.width, canvas.width / aspectRatio);
			context.drawImage(chassisImage, -scaledWidth / 2, -scaledHeight / 2, scaledWidth, scaledHeight);
		}
		context.restore();
		drawMotorChassis(propellerImage, - (scaledWidth / 2), - (scaledHeight / 2), scaledWidthPropeller, scaledHeightPropeller, -rotationAngle);
		drawMotorChassis(propellerImage, (scaledWidth / 2), - (scaledHeight / 2), scaledWidthPropeller, scaledHeightPropeller, rotationAngle);
		drawMotorChassis(propellerImage, (scaledWidth / 2), (scaledHeight / 2), scaledWidthPropeller, scaledHeightPropeller, -rotationAngle);
		drawMotorChassis(propellerImage, -(scaledWidth / 2), (scaledHeight / 2), scaledWidthPropeller, scaledHeightPropeller, rotationAngle);
		//drawMotorChassis(propellerImage, -scaledWidthPropeller / 2 - (scaledWidth / 2), -scaledHeightPropeller / 2 + (scaledHeight / 2), scaledWidthPropeller, scaledHeightPropeller, -rotationAngle);
		//drawMotorChassis(propellerImage, -scaledWidthPropeller / 2 + (scaledWidth / 2), -scaledHeightPropeller / 2 - (scaledHeight / 2), scaledWidthPropeller, scaledHeightPropeller, rotationAngle);
		//drawMotorChassis(propellerImage, -scaledWidthPropeller / 2 + (scaledWidth / 2), -scaledHeightPropeller / 2 + (scaledHeight / 2), scaledWidthPropeller, scaledHeightPropeller, -rotationAngle);


		// Call the animate function again before the next repaint
		requestAnimationFrame(animate);
	}
	animate();

}

function QuadcopterAnimateleftRight(canvas_id, quadcopterImageSrc) {
	const canvas = document.getElementById(canvas_id);
	const context = canvas.getContext("2d");

	const image = new Image();
	image.src = quadcopterImageSrc;

	//scale image
	const scaleFactor = 0.12; // Scale factor to half the size
	const scaledWidth = image.width * scaleFactor;
	const scaledHeight = image.height * scaleFactor;

	image.addEventListener("load", () => {
		// Initial position of the image at the center of the canvas
		let x = 0;
		const y = (canvas.height - scaledHeight) / 2;
		// Initial rotation angle (in degrees)
		let rotationAngle = 45;
		// Initial movement direction and speed
		let direction = 1; // 1 for right, -1 for left
		const speed = 2; // Change the movement speed here (2 pixels per frame in this example)

		// Function to draw the rotated image at the given position and angle
		function drawRotatedImage() {
			context.clearRect(0, 0, canvas.width, canvas.height);

			context.save();

			// Set the transform origin to the center of the image
			context.translate(x + scaledWidth/2, canvas.height / 2);
			context.rotate((-45 * Math.PI) / 180); // Convert degrees to radians
			//context.translate(x + scaledWidth/2, canvas.height/2 + scaledHeight / 2);
			//context.rotate((rotationAngle * Math.PI) / 180); // Convert degrees to radians

			// Draw the image with the top-left corner at the center
			context.drawImage(image, -scaledWidth/2, -scaledHeight / 2, scaledWidth, scaledHeight);

			context.restore();
		}

		// Function to update the image position
		function updatePosition() {
			x += (speed * direction);
			// Check if the image has reached the left or right side of the canvas
			if (x < 0) {
				direction = 1; // Reverse the direction when reaching the canvas boundaries
				rotationAngle = 45;
				//rotationAngle *= -1; // Reverse the rotation direction
				x = 0;
			}
			if ((x + scaledWidth) > (canvas.width)) {
				direction = -1; // Reverse the direction when reaching the canvas boundaries
				rotationAngle = -45;
				//rotationAngle *= -1; // Reverse the rotation direction
				x = canvas.width - scaledWidth;
			}
			drawRotatedImage(); // Redraw the image with the updated position
			requestAnimationFrame(updatePosition);
		}

		// Start the animation
		updatePosition();
	});
}

function canvas6() {
	const canvas = document.getElementById("myCanvas6");
	const context = canvas.getContext("2d");
	const image = new Image();

	// Replace 'path_to_your_image' with the actual path to your image
	image.src = quadcopter_front_path;

		//scale image
	const scaleFactor = 0.12; // Scale factor to half the size
	const scaledWidth = image.width * scaleFactor;
	const scaledHeight = image.height * scaleFactor;

	// Wait for the image to load before drawing it on the canvas
	image.addEventListener("load", () => {
		// Initial position of the image at the center of the canvas
		const y = (canvas.height - scaledHeight) / 2;

		// Initial rotation angle (in degrees)
		let rotationAngle = 45;

		// Initial x position off-canvas to the left
		let x = -scaledWidth;

		// Initial movement direction and speed
		let direction = 1; // 1 for right, -1 for left
		const speed = 2; // Change the movement speed here (2 pixels per frame in this example)

		// Function to draw the rotated image at the given position and angle
		function drawRotatedImage() {
			context.clearRect(0, 0, canvas.width, canvas.height);

			context.save();

			// Set the transform origin to the center of the image
			context.translate(x + scaledWidth / 2, canvas.height / 2);
			context.rotate((rotationAngle * Math.PI) / 180); // Convert degrees to radians

			// Draw the image with the top-left corner at the center
			context.drawImage(image, -scaledWidth / 2, -scaledHeight / 2, scaledWidth, scaledHeight);

			context.restore();
		}

		// Function to update the image position
		function updatePosition() {
			x += speed * direction;

			// Check if the image has reached the left or right side of the canvas
			if (x < 0 || x >= canvas.width - scaledWidth) {
				direction *= -1; // Reverse the direction when reaching the canvas boundaries
				rotationAngle *= -1; // Reverse the rotation direction
				x = Math.min(Math.max(x, 0), canvas.width - scaledWidth); // Clamp the x position to the canvas boundaries

			}

			drawRotatedImage(); // Redraw the image with the updated position
			requestAnimationFrame(updatePosition);
		}

		// Start the animation
		updatePosition();
	});
}

