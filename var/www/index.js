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

function createNavbar() {
	window.addEventListener("DOMContentLoaded", (event) => {
		const navbar = document.createElement('div');
		navbar.classList.add('navbar');

		const container = document.createElement('div');
		container.classList.add('container');
		navbar.append(container);

		const list = document.createElement('ul');
		container.append(list);

		const appendLogo = () => {
			const image = document.createElement('img');
			image.src = '/images/42-logo.png';

			const anchor = document.createElement('a');
			anchor.href = '/';
			anchor.append(image);

			const listItem = document.createElement('li');
			listItem.append(anchor);
			listItem.classList.add('logo');
			list.append(listItem);
		}

		const appendListItem = (name) => {
			const anchor = document.createElement('a');
			anchor.classList.add('button');
			anchor.innerText = name.toUpperCase();
			anchor.href = `/index-${name}.html`;

			const listItem = document.createElement('li');
			listItem.append(anchor);
			list.append(listItem);
		}

		appendLogo();
		appendListItem('alpha');
		appendListItem('beta');

		document.querySelector('body').prepend(navbar);
	});
}

createRunningButton();

createNavbar();
