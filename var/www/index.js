function sayHello(message) {
	alert(message || 'HELLO WORLD');
}

function appendRunningButton() {
	window.addEventListener("DOMContentLoaded", (event) => {
		const runningContainer = document.createElement('div');
		runningContainer.classList.add('running-container');

		const runningButton = document.createElement('button');
		runningButton.innerText = 'TRY ME';
		runningButton.onclick = () => { sayHello('CAUGHT ME :(') };
		runningButton.classList.add('running-button');

		runningButton.style.top = '1%';
		runningButton.style.left = '1%';

		runningContainer.addEventListener("mouseover", (event) => {
			runningButton.style.top = `max(1%, calc(${(Math.random() * 100)}% - 45px - 1%))`;
			runningButton.style.left = `max(1%, calc(${(Math.random() * 100)}% - 120px - 1%))`;
		});

		runningContainer.append(runningButton);
		document.querySelector('.page-wrapper .container').append(runningContainer);
	});
}

function appendReturnButtons() {
	window.addEventListener("DOMContentLoaded", (event) => {
		const backToAlpha = document.createElement('a');
		backToAlpha.innerText = 'ALPHA';
		backToAlpha.href = 'index-alpha.html';
		backToAlpha.classList.add('back-button');

		const backToBeta = document.createElement('a');
		backToBeta.innerText = 'BETA';
		backToBeta.href = 'index-beta.html';
		backToBeta.classList.add('back-button');

		const buttons = document.createElement('div');
		buttons.classList.add('back-buttons');
		buttons.append(backToAlpha, backToBeta);

		document.querySelector('.page-wrapper .container').append(buttons);
	});
}

if (window.location.pathname == '/index-alpha.html') {
	appendRunningButton();
}

appendReturnButtons();
