<main>
  <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.3/dist/leaflet.css" integrity="sha256-kLaT2GOSpHechhsozzB+flnD+zUyjE2LlfWPgU04xyI=" crossorigin="" />

  <div bind:this={mapElement}></div>

  <div class="bg-blue-300 border-black-600 border-b p-4 m-4 rounded">
    This is a map.
  </div>
</main>

<style>
  @tailwind utilities;
  @tailwind components;
  @tailwind base;
  /*@import 'leaflet/dist/leaflet.css';*/
  main div {
        width: 800px;
        height: 500px;
  }
</style>

<script>
    import { onMount, onDestroy } from 'svelte';
    //import { browser } from '$app/env';

    let mapElement;
    let map;

    onMount(async () => {
        const leaflet = await import('leaflet');

        map = leaflet.map(mapElement).setView([51.505, -0.09], 13);

        leaflet.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
            attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
        }).addTo(map);

        leaflet.marker([51.5, -0.09]).addTo(map)
            .bindPopup('A pretty CSS3 popup.<br> Easily customizable.')
            .openPopup();
    });

    onDestroy(async () => {
        if(map) {
            console.log('Unloading Leaflet map.');
            map.remove();
        }
    });
</script>
