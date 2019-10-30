/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "export/view/export_view_content.h"

#include "lang/lang_keys.h"
#include "layout.h"

namespace Export {
namespace View {

const QString Content::kDoneId = "done";

Content ContentFromState(const ProcessingState &state) {
	using Step = ProcessingState::Step;

	auto result = Content();
	const auto push = [&](
			const QString &id,
			const QString &label,
			const QString &info,
			float64 progress) {
		result.rows.push_back({ id, label, info, progress });
	};
	const auto pushMain = [&](const QString &label) {
		const auto info = (state.entityCount > 0)
			? (QString::number(state.entityIndex + 1)
				+ " / "
				+ QString::number(state.entityCount))
			: QString();
		if (!state.substepsTotal) {
			push("main", label, info, 0.);
			return;
		}
		const auto substepsTotal = state.substepsTotal;
		const auto step = static_cast<int>(state.step);
		const auto done = state.substepsPassed;
		const auto add = state.substepsNow;
		const auto doneProgress = done / float64(substepsTotal);
		const auto addPart = [&](int index, int count) {
			return (count > 0)
				? ((float64(add) * index)
					/ (float64(substepsTotal) * count))
				: 0.;
		};
		const auto addProgress = (state.entityCount == 1
			&& !state.entityIndex)
			? addPart(state.itemIndex, state.itemCount)
			: addPart(state.entityIndex, state.entityCount);
		push("main", label, info, doneProgress + addProgress);
	};
	const auto pushBytes = [&](const QString &id, const QString &label) {
		if (!state.bytesCount) {
			return;
		}
		const auto progress = state.bytesLoaded / float64(state.bytesCount);
		const auto info = formatDownloadText(
			state.bytesLoaded,
			state.bytesCount);
		push(id, label, info, progress);
	};
	switch (state.step) {
	case Step::Initializing:
		pushMain(tr::lng_export_state_initializing(tr::now));
		break;
	case Step::DialogsList:
		pushMain(tr::lng_export_state_chats_list(tr::now));
		break;
	case Step::PersonalInfo:
		pushMain(tr::lng_export_option_info(tr::now));
		break;
	case Step::Userpics:
		pushMain(tr::lng_export_state_userpics(tr::now));
		pushBytes(
			"userpic" + QString::number(state.entityIndex),
			state.bytesName);
		break;
	case Step::Contacts:
		pushMain(tr::lng_export_option_contacts(tr::now));
		break;
	case Step::Sessions:
		pushMain(tr::lng_export_option_sessions(tr::now));
		break;
	case Step::OtherData:
		pushMain(tr::lng_export_option_other(tr::now));
		break;
	case Step::Dialogs:
		pushMain(tr::lng_export_state_chats(tr::now));
		push(
			"chat" + QString::number(state.entityIndex),
			(state.entityName.isEmpty()
				? tr::lng_deleted(tr::now)
				: (state.entityType == ProcessingState::EntityType::Chat)
				? state.entityName
				: tr::lng_saved_messages(tr::now)),
			(state.itemCount > 0
				? (QString::number(state.itemIndex)
					+ " / "
					+ QString::number(state.itemCount))
				: QString()),
			(state.itemCount > 0
				? (state.itemIndex / float64(state.itemCount))
				: 0.));
		pushBytes(
			("file"
				+ QString::number(state.entityIndex)
				+ '_'
				+ QString::number(state.itemIndex)),
			state.bytesName);
		break;
	default: Unexpected("Step in ContentFromState.");
	}
	while (result.rows.size() < 3) {
		result.rows.emplace_back();
	}
	return result;
}

Content ContentFromState(const FinishedState &state) {
	auto result = Content();
	result.rows.push_back({
		Content::kDoneId,
		tr::lng_export_finished(tr::now),
		QString(),
		1. });
	result.rows.push_back({
		Content::kDoneId,
		tr::lng_export_total_amount(
			tr::now,
			lt_amount,
			QString::number(state.filesCount)),
		QString(),
		1. });
	result.rows.push_back({
		Content::kDoneId,
		tr::lng_export_total_size(
			tr::now,
			lt_size,
			formatSizeText(state.bytesCount)),
		QString(),
		1. });
	return result;
}

} // namespace View
} // namespace Export
