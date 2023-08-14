function sayHello(message) {
	alert(message || 'HELLO WORLD');
}

function createRunningButton() {
	window.addEventListener("DOMContentLoaded", (event) => {
		const section = document.querySelector('.running-button-section');

		if ( !section ) return;

		const container = document.createElement('div');
		container.classList.add('running-container');

		const button = document.createElement('button');
		button.innerText = 'TRY ME';
		button.onclick = () => { sayHello('CAUGHT ME :(') };
		button.classList.add('running-button');

		button.style.top = '1%';
		button.style.left = '1%';

		container.addEventListener("mouseover", (event) => {
			button.style.top = `max(1%, calc(${(Math.random() * 100)}% - 45px - 1%))`;
			button.style.left = `max(1%, calc(${(Math.random() * 100)}% - 120px - 1%))`;
		});

		container.append(button);
		section.append(container);
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

createRunningButton();

appendReturnButtons();
