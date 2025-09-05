class PackageDownloadCallbacks(libdnf5.repo.DownloadCallbacks):
    # List of descriptions of downloads.
    download_descriptions = []

    def add_new_download(self, user_data, description, total_to_download):
        print(f"Started downloading: {description}, size: {total_to_download}")

        # Store the message describing the new download and return its index in the list.
        self.download_descriptions.append(description)
        return len(self.download_descriptions) - 1

    def end(self, user_cb_data, status, msg):
        # Get the description based on the index passed in user_cb_data.
        description = self.download_descriptions[user_cb_data]

        if status is libdnf5.repo.DownloadCallbacks.TransferStatus_SUCCESSFUL:
            print(f"Downloaded: {description}")
        elif status is libdnf5.repo.DownloadCallbacks.TransferStatus_ALREADYEXISTS:
            print(f"Skipped to download: {description}: {msg}")
        else:
            print(f"Failed to download: {description}: {msg}")

        return 0


# Set the download callbacks.
downloader_callbacks = PackageDownloadCallbacks()
base.set_download_callbacks(
    libdnf5.repo.DownloadCallbacksUniquePtr(downloader_callbacks))

# Download the packages.
transaction.download()
