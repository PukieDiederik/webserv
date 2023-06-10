function sayHello() {
	alert("HELLO WORLD");
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

appendReturnButtons();
