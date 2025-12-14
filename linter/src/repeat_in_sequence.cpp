#include "repeat_in_sequence.h"

#include <cstdint>
#include <string>

#include "Surelog/Design/Design.h"
#include "Surelog/Design/FileContent.h"
#include "Surelog/ErrorReporting/ErrorContainer.h"
#include "Surelog/SourceCompile/SymbolTable.h"
#include "Surelog/SourceCompile/VObjectTypes.h"
#include "linter_utils.h"

using namespace SURELOG;

namespace Analyzer {

// Inside one Sequence_expr there should not be both
// paGoto_repetition and paNon_consecutive_repetition
void checkRepetitionInSequence(const FileContent* fC, ErrorContainer* errors,
                               SymbolTable* symbols) {
  if (!fC) return;

  NodeId root = fC->getRootNode();
  if (!root) return;

  // Find all Sequence_declaration
  auto seqDecls = fC->sl_collect_all(root, VObjectType::paSequence_declaration);

  for (NodeId seqDeclId : seqDecls) {
    // Sequence name (if available)
    std::string seqName = extractName(fC, seqDeclId);

    // Find all Sequence_expr in this sequence_declaration
    auto seqExprs = fC->sl_collect_all(seqDeclId, VObjectType::paSequence_expr);

    for (NodeId seqExprId : seqExprs) {
      // Find different types of repetitions inside seqExpr
      auto gotoNodes =
          fC->sl_collect_all(seqExprId, VObjectType::paGoto_repetition);
      auto nonConsecNodes = fC->sl_collect_all(
          seqExprId, VObjectType::paNon_consecutive_repetition);

      bool hasGoto = !gotoNodes.empty();
      bool hasNonConsec = !nonConsecNodes.empty();

      if (hasGoto && hasNonConsec) {
        reportError(fC, seqExprId, seqName,
                    ErrorDefinition::LINT_REPETITION_IN_SEQUENCE, errors,
                    symbols);
      }
    }
  }
}

}  // namespace Analyzer
