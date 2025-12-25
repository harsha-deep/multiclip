<script>
    import { onMount } from "svelte";
    import {
        GetHistory,
        Copy,
        ClearHistory,
        DeleteItem,
        PinItem,
    } from "../wailsjs/go/main/App";
    import { WindowMinimise, Quit } from "../wailsjs/runtime/runtime";

    let items = [];
    let menuIndex = null;

    async function refresh() {
        items = await GetHistory();
    }

    async function clear() {
        await ClearHistory();
        items = [];
    }

    async function deleteItem(index) {
        await DeleteItem(index);
        items = await GetHistory();
        menuIndex = null;
    }

    async function pinItem(index) {
        await PinItem(index);
        items = await GetHistory();
        menuIndex = null;
    }

    onMount(refresh);
    setInterval(refresh, 800);
</script>

<main>
    <div class="titlebar">
        <span>Multi Clipboard</span>
        <button on:click={() => WindowMinimise()}>—</button>
        <button on:click={() => Quit()}>✕</button>
    </div>

    <!-- <button class="clear-button" on:click={clear}>Clear</button> -->

    <ul class="list">
        {#each items as item, i}
            <li class="item">
                <div class="box">
                    <button class="clipboard-item" on:click={() => Copy(item)}>
                        <span>{item}</span>
                    </button>

                    <button
                        class="menu-btn"
                        on:click={() =>
                            (menuIndex = menuIndex === i ? null : i)}
                    >
                        ⋮
                    </button>

                    {#if menuIndex === i}
                        <div class="menu">
                            <button on:click={() => deleteItem(i)}>Delete</button>
                            <button on:click={() => pinItem(i)}>Pin</button>
                            <hr />
                            <button on:click={clear}>Clear All</button>
                        </div>
                    {/if}
                </div>
            </li>
        {/each}
    </ul>
</main>

<style>
    .titlebar {
        display: flex;
        justify-content: space-between;
        align-items: center;
        padding: 6px 10px;
        background: #111;
        color: white;
        -webkit-app-region: drag;
    }

    .titlebar button {
        -webkit-app-region: no-drag;
        background: none;
        border: none;
        color: white;
        cursor: pointer;
    }

    .list {
        list-style: none;
        padding: 0;
        margin: 0;
    }

    .item {
        margin-bottom: 8px;
    }

    .clipboard-item {
        height: 60px;
        width: 100%;
        background: #222;
        color: white;
        border: none;
        border-radius: 6px;
        display: flex;
        align-items: center;
        justify-content: center;
        padding: 10px;
        cursor: pointer;
        overflow: hidden;
    }

    .clipboard-item span {
        text-align: center;
        overflow: hidden;
        text-overflow: ellipsis;
        display: -webkit-box;
        -webkit-box-orient: vertical;
    }

    .box {
        position: relative;
    }

    .menu-btn {
        position: absolute;
        top: 6px;
        right: 6px;
        background: none;
        border: none;
        color: #aaa;
        cursor: pointer;
    }

    .menu {
        position: absolute;
        top: 28px;
        right: 6px;
        background: #333;
        border-radius: 6px;
        padding: 6px 0;
        width: 120px;
        z-index: 10;
    }

    .menu button {
        padding: 6px 12px;
        cursor: pointer;
    }

    .menu button:hover {
        background: #444;
    }

    .menu hr {
        border: none;
        border-top: 1px solid #555;
        margin: 4px 0;
    }
</style>
