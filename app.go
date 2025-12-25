package main

import (
	"context"
)

// App struct
type App struct {
	ctx context.Context
}

// NewApp creates a new App application struct
func NewApp() *App {
	return &App{}
}

// startup is called when the app starts. The context is saved
// so we can call the runtime methods
func (a *App) startup(ctx context.Context) {
	a.ctx = ctx
	StartClipboardWatcher()
}

func (a *App) GetHistory() []string {
	return GetClipboardHistory()
}

func (a *App) Copy(text string) {
	CopyToClipboard(text)
}

func (a *App) ClearHistory() {
	ClearClipboardHistory()
}

func (a *App) DeleteItem(index int) {
	DeleteItem(index)
}

func (a *App) PinItem(index int) {
	PinItem(index)
}
