import AppKit
import Foundation

final class ScratchPadInstallerApp: NSObject, NSApplicationDelegate, NSWindowDelegate {
    private var window: NSWindow!
    private var vst3Checkbox: NSButton!
    private var auCheckbox: NSButton!
    private var installButton: NSButton!
    private var progressIndicator: NSProgressIndicator!
    private var statusLabel: NSTextField!
    private var activeProcess: Process?
    private var choicesFileURL: URL?

    func applicationDidFinishLaunching(_ notification: Notification) {
        NSApp.setActivationPolicy(.regular)
        createWindow()
        window.center()
        window.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        true
    }

    func windowShouldClose(_ sender: NSWindow) -> Bool {
        activeProcess == nil
    }

    private func createWindow() {
        window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 480, height: 300),
            styleMask: [.titled, .closable],
            backing: .buffered,
            defer: false
        )
        window.title = "ScratchPad Installer"
        window.isReleasedWhenClosed = false
        window.delegate = self

        let title = NSTextField(labelWithString: "Choose plugin formats")
        title.font = .boldSystemFont(ofSize: 18)

        let description = NSTextField(wrappingLabelWithString:
            "Select the formats you want to install. At least one format must be selected.")
        description.maximumNumberOfLines = 0

        vst3Checkbox = NSButton(checkboxWithTitle: "VST3 — Cubase and other VST3-compatible DAWs",
                                target: nil,
                                action: nil)
        vst3Checkbox.state = .on

        auCheckbox = NSButton(checkboxWithTitle: "Audio Unit (AU) — Logic Pro and other AU-compatible DAWs",
                              target: nil,
                              action: nil)
        auCheckbox.state = .on

        let formats = NSStackView(views: [vst3Checkbox, auCheckbox])
        formats.orientation = .vertical
        formats.alignment = .leading
        formats.spacing = 12

        installButton = NSButton(title: "Install", target: self, action: #selector(installSelectedFormats))
        installButton.keyEquivalent = "\r"

        progressIndicator = NSProgressIndicator()
        progressIndicator.style = .spinning
        progressIndicator.controlSize = .small
        progressIndicator.isDisplayedWhenStopped = false
        progressIndicator.isHidden = true

        statusLabel = NSTextField(wrappingLabelWithString: "")
        statusLabel.maximumNumberOfLines = 0

        let statusRow = NSStackView(views: [progressIndicator, statusLabel])
        statusRow.orientation = .horizontal
        statusRow.alignment = .centerY
        statusRow.spacing = 8

        let content = NSStackView(views: [title, description, formats, statusRow, installButton])
        content.orientation = .vertical
        content.alignment = .leading
        content.spacing = 16
        content.translatesAutoresizingMaskIntoConstraints = false

        window.contentView?.addSubview(content)
        NSLayoutConstraint.activate([
            content.leadingAnchor.constraint(equalTo: window.contentView!.leadingAnchor, constant: 24),
            content.trailingAnchor.constraint(equalTo: window.contentView!.trailingAnchor, constant: -24),
            content.topAnchor.constraint(equalTo: window.contentView!.topAnchor, constant: 24),
            content.bottomAnchor.constraint(lessThanOrEqualTo: window.contentView!.bottomAnchor, constant: -24),
            statusLabel.widthAnchor.constraint(equalTo: content.widthAnchor, constant: -24)
        ])
    }

    @objc private func installSelectedFormats() {
        let installVST3 = vst3Checkbox.state == .on
        let installAU = auCheckbox.state == .on

        guard installVST3 || installAU else {
            statusLabel.stringValue = "Select at least one plugin format to continue."
            return
        }

        guard let packageURL = Bundle.main.resourceURL?.appendingPathComponent("ScratchPad.pkg") else {
            statusLabel.stringValue = "The installer package could not be found."
            return
        }

        do {
            let choicesToToggle = [
                installVST3 ? nil : "com.ninelivesaudio.scratchpad.vst3",
                installAU ? nil : "com.ninelivesaudio.scratchpad.au"
            ].compactMap { $0 }
            let choicesData = try PropertyListSerialization.data(
                fromPropertyList: choicesToToggle,
                format: .xml,
                options: 0
            )
            let choicesURL = FileManager.default.temporaryDirectory
                .appendingPathComponent("ScratchPadInstallChoices-\(UUID().uuidString).plist")
            try choicesData.write(to: choicesURL, options: .atomic)
            choicesFileURL = choicesURL

            startInstaller(packageURL: packageURL, choicesURL: choicesURL)
        } catch {
            statusLabel.stringValue = "Could not prepare the installer: \(error.localizedDescription)"
        }
    }

    private func startInstaller(packageURL: URL, choicesURL: URL) {
        let process = Process()
        process.executableURL = URL(fileURLWithPath: "/usr/bin/osascript")
        process.arguments = [
            "-e",
            """
            on run argv
                set packagePath to item 1 of argv
                set choicesPath to item 2 of argv
                set installCommand to "/usr/sbin/installer -allowUntrusted -pkg " & quoted form of packagePath & " -target / -applyChoiceChangesXML " & quoted form of choicesPath
                with timeout of 3600 seconds
                    do shell script installCommand with administrator privileges
                end timeout
            end run
            """,
            packageURL.path,
            choicesURL.path
        ]
        process.standardOutput = FileHandle.nullDevice
        process.standardError = Pipe()
        process.terminationHandler = { [weak self] process in
            let errorData = (process.standardError as? Pipe)?.fileHandleForReading.readDataToEndOfFile()
            let errorMessage = errorData
                .flatMap { String(data: $0, encoding: .utf8) }
                .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
            DispatchQueue.main.async {
                self?.installerFinished(
                    success: process.terminationStatus == 0,
                    errorMessage: errorMessage
                )
            }
        }

        do {
            try process.run()
            activeProcess = process
            setInstalling(true)
            statusLabel.stringValue = "Installing selected formats. You may be asked to enter your administrator password."
        } catch {
            let cleanupError = removeChoicesFile()
            let cleanupMessage = cleanupError.map { " \($0)" } ?? ""
            statusLabel.stringValue = "Could not start the installer: \(error.localizedDescription).\(cleanupMessage)"
        }
    }

    private func setInstalling(_ installing: Bool) {
        vst3Checkbox.isEnabled = !installing
        auCheckbox.isEnabled = !installing
        installButton.isEnabled = !installing
        progressIndicator.isHidden = !installing

        if installing {
            progressIndicator.startAnimation(nil)
        } else {
            progressIndicator.stopAnimation(nil)
        }
    }

    private func installerFinished(success: Bool, errorMessage: String?) {
        setInstalling(false)
        activeProcess = nil

        let cleanupError = removeChoicesFile()

        if success {
            if let cleanupError {
                statusLabel.stringValue = "ScratchPad was installed successfully, but \(cleanupError)"
            } else {
                statusLabel.stringValue = "ScratchPad was installed successfully."
            }
        } else {
            let detail: String
            if let errorMessage, !errorMessage.isEmpty {
                detail = errorMessage
            } else {
                detail = "You can try again."
            }
            let cleanupMessage = cleanupError.map { " \($0)" } ?? ""
            statusLabel.stringValue = "Installation was cancelled or failed. \(detail)\(cleanupMessage)"
        }
    }

    private func removeChoicesFile() -> String? {
        guard let choicesFileURL else {
            return nil
        }

        do {
            try FileManager.default.removeItem(at: choicesFileURL)
            self.choicesFileURL = nil
            return nil
        } catch {
            return "Could not remove the temporary installer options file: \(error.localizedDescription)"
        }
    }
}

let application = NSApplication.shared
let delegate = ScratchPadInstallerApp()
application.delegate = delegate
application.run()
