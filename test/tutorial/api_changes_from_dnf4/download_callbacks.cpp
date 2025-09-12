/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
# include <libdnf5/repo/download_callbacks.hpp>
*/

class PackageDownloadCallbacks : public libdnf5::repo::DownloadCallbacks {
private:
    using TransferStatus = libdnf5::repo::DownloadCallbacks::TransferStatus;

    /// List of descriptions of downloads.
    std::forward_list<std::string> download_descriptions;

    void * add_new_download([[maybe_unused]] void * user_data, const char * description, double total_to_download) {
        std::cout << "Started downloading: " << description << ", size: " << total_to_download << std::endl;

        // Store the message describing the new download and return a pointer to it.
        return &download_descriptions.emplace_front(description);
    }

    int end(void * user_cb_data, TransferStatus status, const char * msg) {
        // Check that user_cb_data is present in download_descriptions.
        std::string * description{nullptr};
        for (const auto & item : download_descriptions) {
            if (&item == user_cb_data) {
                // Get the description from the user_cb_data.
                description = reinterpret_cast<std::string *>(user_cb_data);
                break;
            }
        }
        if (!description) {
            return 0;
        }

        std::string message;
        switch (status) {
            case TransferStatus::SUCCESSFUL:
                std::cout << "  Downloaded: " << *description << std::endl;
                break;
            case TransferStatus::ALREADYEXISTS:
                std::cout << "  Already downloaded: " << *description << std::endl;
                break;
            case TransferStatus::ERROR:
                std::cout << "  Error downloading: " << *description << ": " << msg << std::endl;
                break;
        }

        return 0;
    }
};

// Set the download callbacks.
base.set_download_callbacks(std::make_unique<PackageDownloadCallbacks>());

// Download the packages.
transaction.download();
