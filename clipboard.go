package main

import (
	"sync"
	"time"

	"github.com/atotto/clipboard"
)

var (
	history  []string
	last     string
	lock     sync.Mutex
	maxItems = 10
)

func StartClipboardWatcher() {
	go func() {
		for {
			text, err := clipboard.ReadAll()
			if err == nil && text != "" && text != last {
				lock.Lock()
				last = text
				history = append([]string{text}, history...)
				if len(history) > maxItems {
					history = history[:maxItems]
				}
				lock.Unlock()
			}
			time.Sleep(500 * time.Millisecond)
		}
	}()
}

func GetClipboardHistory() []string {
	lock.Lock()
	defer lock.Unlock()
	return history
}

func CopyToClipboard(text string) {
	clipboard.WriteAll(text)
}
