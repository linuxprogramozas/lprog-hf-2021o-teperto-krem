function createElementFromHTML(html) {
    const virtualElement = document.createElement("div");
    virtualElement.innerHTML = html;
    return virtualElement.childElementCount == 1 ? virtualElement.firstChild : virtualElement.children;
}

function handleResponse(response) {
    if (response.hasOwnProperty('x') && response.hasOwnProperty('y')) {
        let field = fieldAt(response.x, response.y)
        field.appendChild(createElementFromHTML(
            `<img src="o.png"/>`
        ))
        field.onclick = undefined
    }
    if (response.status == "victory") {
        alert("victory")
    } else if (response.status == "defeat") {
        alert("defeat")
    }
}

function placeAt(x, y) {
    let data = {
        'x': x,
        'y': y
    }
    let formBody = []
    for (let property in data) {
        let encodedKey = encodeURIComponent(property)
        let encodedValue = encodeURIComponent(data[property])
        formBody.push(encodedKey + "=" + encodedValue)
    }
    formBody = formBody.join("&")
    fetch(window.location.origin + "/board/place", {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded;charset=UTF-8'
        },
        body: formBody
    })
        .then(response => response.json())
        .then(json => handleResponse(json))
}

let fields = []

function fieldAt(x, y) {
    return fields[x + y * 3]
}

for (let field of document.querySelectorAll(".field")) {
    fields.push(field)
}

function setOnclick() {
    for (let x = 0; x < 3; ++x) {
        for (let y = 0; y < 3; ++y) {
            fieldAt(x, y).onclick = () => {
                fieldAt(x, y).appendChild(createElementFromHTML(
                    `<img src="x.png">`
                ))
                placeAt(x, y)
                fieldAt(x, y).onclick = undefined
            }
        }
    }
}
setOnclick()

function getBoard() {
    fetch(window.location.origin + "/board")
        .then(response => response.json())
        .then(board => {
            for (let i in board) {
                fields[i].innerHTML = ''
                if (board[i] != " ") {
                    fields[i].appendChild(createElementFromHTML(
                        `<img src="${board[i]}.png"/>`
                    ))
                    fields[i].onclick = undefined
                }
            }
            setOnclick();
        })
}

getBoard()

let reset_button = document.querySelector("#reset_button")
reset_button.onclick = () => {
    fetch(window.origin + "/board/reset", {
        method: 'POST',
    })
        .then(response => response.status)
        .then(status => {
            if (status == 200) {
                getBoard()
            }
        })
}