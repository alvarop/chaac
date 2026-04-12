// Shared layout logic for Chaac pages
// Extracts host from URL path and builds navbar + settings panel

function getChaacHost() {
  var parts = window.location.pathname.split('/').filter(Boolean);
  return parts.length > 0 ? parts[0] : null;
}

function initLayout() {
  var host = getChaacHost();

  // Build navbar
  var navMenu = document.getElementById('navbarMenu');
  if (host && navMenu) {
    navMenu.innerHTML =
      '<div class="navbar-start">' +
        '<a class="navbar-item" href="/' + host + '">' + host + ' - Latest</a>' +
        '<a class="navbar-item" href="/' + host + '/plots">Plots</a>' +
        '<a class="navbar-item" href="/' + host + '/stats">Stats</a>' +
        '<a class="navbar-item" id="settings_button">Settings</a>' +
      '</div>';

    // Show settings panel
    document.getElementById('settings').style.display = 'none';
    document.getElementById('settings_button').addEventListener('click', function() {
      document.getElementById('settings').style.display = 'block';
    });
  } else {
    // Landing page — hide settings entirely
    var settings = document.getElementById('settings');
    if (settings) settings.style.display = 'none';
  }

  // Burger menu toggle
  var burgers = Array.prototype.slice.call(document.querySelectorAll('.navbar-burger'), 0);
  burgers.forEach(function(el) {
    el.addEventListener('click', function() {
      var target = document.getElementById(el.dataset.target);
      el.classList.toggle('is-active');
      target.classList.toggle('is-active');
    });
  });

  // Settings defaults
  if (localStorage.pressure_units == null) {
    localStorage.pressure_units = 'hPa';
  }
  if (localStorage.temperature_units == null) {
    localStorage.temperature_units = 'c';
  }
  setDefaultRadioButtons();

  document.getElementById('save_settings_btn').addEventListener('click', saveSettings);
  document.getElementById('cancel_settings_btn').addEventListener('click', setDefaultRadioButtons);
}

function setDefaultRadioButtons() {
  if (localStorage.pressure_units == 'hPa') {
    document.getElementById('pressure_units_hpa').checked = true;
  } else {
    document.getElementById('pressure_units_inhg').checked = true;
  }
  if (localStorage.temperature_units == 'c') {
    document.getElementById('temperature_units_c').checked = true;
  } else {
    document.getElementById('temperature_units_f').checked = true;
  }
  document.getElementById('settings').style.display = 'none';
}

function saveSettings() {
  if (document.getElementById('pressure_units_hpa').checked) {
    localStorage.pressure_units = 'hPa';
  } else {
    localStorage.pressure_units = 'inHg';
  }
  if (document.getElementById('temperature_units_c').checked) {
    localStorage.temperature_units = 'c';
  } else {
    localStorage.temperature_units = 'f';
  }
  if (typeof draw_chaac === 'function') {
    draw_chaac();
  }
  document.getElementById('settings').style.display = 'none';
}

document.addEventListener('DOMContentLoaded', initLayout);
