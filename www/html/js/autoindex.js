function deleteFile(path) {
  fetch(path, {
    method: 'DELETE'
  }).then(response => {
    if (response.ok) {
      alert("Archivo eliminado");
      location.reload();
    } else {
      alert("Error al eliminar el archivo");
    }
  });
}

// document.addEventListener("DOMContentLoaded", function () {
//   const uploadForm = document.getElementById("uploadForm");

//   if (uploadForm) {
//     uploadForm.addEventListener("submit", function (e) {
//       e.preventDefault(); // Evita la recarga de página

//       const formData = new FormData(uploadForm);
//       const actionUrl = uploadForm.getAttribute("action");

//       fetch(actionUrl, {
//         method: "POST",
//         body: formData
//       })
//         .then(response => {
//           if (response.ok) {
//             alert("Archivo subido con éxito");
//             location.reload();
//           } else {
//             alert("Error al subir el archivo");
//           }
//         })
//         .catch(error => {
//           console.error("Error:", error);
//           alert("Error en la subida del archivo");
//         });
//     });
//   }
// });
document.addEventListener("DOMContentLoaded", function () {
  const uploadForm = document.getElementById("uploadForm");

  if (uploadForm) {
    uploadForm.addEventListener("submit", function (e) {
      e.preventDefault();

      const fileInput = uploadForm.querySelector('input[type="file"]');
      const file = fileInput.files[0];
      if (!file) {
        alert("Selecciona un archivo");
        return;
      }

      const uploadUri = uploadForm.getAttribute("action") + encodeURIComponent(file.name);

      fetch(uploadUri, {
        method: "POST",
        headers: {
          "Content-Type": file.type || "application/octet-stream",
          "Content-Length": file.size // opcional, el navegador puede añadirlo solo
        },
        body: file
      })
        .then(response => {
          if (response.ok) {
            alert("Archivo subido con éxito");
            location.reload();
          } else {
            alert("Error al subir el archivo");
          }
        })
        .catch(error => {
          console.error("Error:", error);
          alert("Error en la subida del archivo");
        });
    });
  }
});
